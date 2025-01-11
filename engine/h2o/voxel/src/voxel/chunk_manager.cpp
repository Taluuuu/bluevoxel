#include "voxel/chunk_manager.h"

#include "core/engine.h"
#include "core/profiling/scope_timer.h"
#include "voxel/chunk_column_heightmap.h"
#include "voxel/voxel_utils.h"

#include <stack>

namespace h2o
{
    ChunkManager::ChunkManager()
    {
        cells_updated_event<Chunk>().add_listener(m_chunks_updated_handle,
            [this](const CellsUpdatedEvent& event)
            {
                on_chunks_updated(event);
            }
        );

        cells_deleted_event().add_listener(m_chunks_deleted_handle,
            [this](const CellsDeletedEvent& event)
            {
                on_chunks_deleted(event);
            }
        );
    }

    Block ChunkManager::get_block_at(const v3i& block_pos)
    {
        Block block = Block::Air;
        fetch<Chunk>(voxel_utils::block_to_chunk_pos(block_pos),
            [&](const Chunk* chunk)
            {
                if (chunk)
                    block = chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));
            }
        );

        return block;
    }

    bool ChunkManager::set_block_at(const v3i& block_pos, const Block block)
    {
        bool success = false;

        const v3i chunk_pos = voxel_utils::block_to_chunk_pos(block_pos);
        fetch_mut<Chunk>(chunk_pos,
            [&](Chunk* chunk)
            {
                if (chunk)
                {
                    chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
                    success = true;
                }
            }, false, false
        );

        request_lighting_update({ chunk_pos }, 0.0f);

        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        for (i32 k = -1; k <= 1; k++)
        {
            const v3i adj_chunk_pos = chunk_pos + v3i{ i, j, k };
            if (adj_chunk_pos != chunk_pos)
                request_lighting_update({ adj_chunk_pos }, 1.0f);
        }

        return success;
    }

    void ChunkManager::set_player_positions(const std::vector<v3>& player_positions)
    {
        const std::unique_lock lock{ m_player_positions_mutex };
        m_player_positions = player_positions;
    }

    bool ChunkManager::is_chunk_column_generated(const v2i chunk_column_pos) const
    {
        bool is_generated = false;
        view_column<Chunk>(chunk_column_pos,
            [&](const View<Chunk>& chunk_column)
            {
                chunk_column.for_each_cell(
                    [&](const Chunk& chunk, const v3i&)
                    {
                        // TODO: Check all/only one of the chunks in the column, idk
                        is_generated = is_generated || chunk.is_generated();
                    }
                );
            }
        );

        return is_generated;
    }

    bool ChunkManager::is_ready_for_meshing(const v3i& chunk_pos) const
    {
        if (!is_ready_for_lighting_update(chunk_pos))
            return false;

        const std::shared_lock pending_lock{ m_chunk_positions_pending_lighting_update_mutex };
        const std::shared_lock built_lock{ m_built_chunk_lightings_mutex };

        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        for (i32 k = -1; k <= 1; k++)
        {
            const v3i adj_chunk_pos = chunk_pos + v3i{ i, j, k };
            if (adj_chunk_pos.y >= 0 && adj_chunk_pos.y < voxel_constants::vertical_chunk_count)
            {
                if (!m_built_chunk_lightings.contains(adj_chunk_pos))
                    return false;

                if (m_chunk_positions_pending_lighting_update.contains(adj_chunk_pos))
                    return false;
            }
        }

        return true;
    }

    bool ChunkManager::is_ready_for_lighting_update(const v3i& chunk_pos) const
    {
        const std::shared_lock lock{ m_generated_chunks_mutex };

        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        for (i32 k = -1; k <= 1; k++)
        {
            const v3i adj_chunk_pos = chunk_pos + v3i{ i, j, k };
            if (adj_chunk_pos.y >= 0 && adj_chunk_pos.y < voxel_constants::vertical_chunk_count)
            {
                if (!m_generated_chunks.contains(adj_chunk_pos))
                    return false;
            }
        }

        return true;
    }

    bool ChunkManager::is_pending_lighting_update(const v3i& chunk_pos) const
    {
        const std::shared_lock lock{ m_chunk_positions_pending_lighting_update_mutex };
        return m_chunk_positions_pending_lighting_update.contains(chunk_pos);
    }

    void ChunkManager::broadcast_events()
    {
        Grid3D::broadcast_events();

        // Manually broadcast chunk lighting update event
        {
            const std::shared_lock lock{ m_chunk_positions_after_lighting_update_mutex };
            cells_updated_event<ChunkLighting>().broadcast(
                CellsUpdatedEvent{ m_chunk_positions_after_lighting_update });
        }

        {
            const std::unique_lock lock{ m_chunk_positions_after_lighting_update_mutex };
            m_chunk_positions_after_lighting_update.clear();
        }
    }

    std::shared_ptr<Grid3D<8, Chunk, ChunkLighting>::CellColumnTuple> ChunkManager::create_cell_column(
        const v2i cell_column_pos)
    {
        const auto chunk_column_tuple = Grid3D::create_cell_column(cell_column_pos);
        auto& chunk_column = std::get<std::array<CellData<Chunk>, voxel_constants::vertical_chunk_count>>(*chunk_column_tuple);

        const auto heightmap = std::make_shared<ChunkColumnHeightmap>();
        for (auto& chunk : chunk_column)
            chunk.cell.set_heightmap(heightmap);

        return chunk_column_tuple;
    }

    void ChunkManager::request_lighting_update(std::unordered_set<v3i> chunks_to_update_lighting, const f32 priority_override)
    {
        {
            const std::unique_lock lock{ m_chunk_positions_pending_lighting_update_mutex };

            for (const v3i& chunk_pos : m_chunk_positions_pending_lighting_update)
                chunks_to_update_lighting.erase(chunk_pos);

            m_chunk_positions_pending_lighting_update.insert(
                chunks_to_update_lighting.begin(),
                chunks_to_update_lighting.end());
        }

        if (chunks_to_update_lighting.empty())
            return;

        f32 priority = FLT_MAX;
        if (priority_override < 0.0f)
        {
            const std::shared_lock lock{ m_player_positions_mutex };
            for (const v3i& chunk_pos : chunks_to_update_lighting)
            {
                const v3 chunk_world_pos = voxel_utils::chunk_to_world_pos(chunk_pos);
                for (const v3& player_pos : m_player_positions)
                {
                    const f32 player_distance = glm::distance(player_pos, chunk_world_pos);
                    priority = glm::min(priority, player_distance);
                }
            }
        }
        else
        {
            priority = priority_override;
        }

        g_engine->thread_pool().queue_job(priority,
            [this, chunks_to_update_lighting]
            {
                for (const v3i& chunk_pos : chunks_to_update_lighting)
                {
                    view<Chunk>(chunk_pos - v3i{1}, v3i{3},
                        [&](const Chunk::ViewType& chunk_view)
                        {
                            fetch_mut<ChunkLighting>(chunk_pos,
                                [&](ChunkLighting* lighting)
                                {
                                    if (lighting)
                                        chunk_lighting::update_lighting(*lighting, chunk_view);

                                    {
                                        const std::unique_lock lock{ m_chunk_positions_pending_lighting_update_mutex };
                                        m_chunk_positions_pending_lighting_update.erase(chunk_pos);
                                    }

                                    {
                                        const std::unique_lock lock{ m_built_chunk_lightings_mutex };
                                        m_built_chunk_lightings.insert(chunk_pos);
                                    }
                                }, false, false
                            );
                        }
                    );
                }

                {
                    const std::unique_lock lock{ m_chunk_positions_after_lighting_update_mutex };

                    // Haven't decided if this is too hacky just yet.
                    // Make sure chunks start rebuilding their mesh once their lightings
                    // are ALL rebuilt.
                    m_chunk_positions_after_lighting_update.insert(
                        chunks_to_update_lighting.begin(),
                        chunks_to_update_lighting.end());
                }
            }
        );
    }

    void ChunkManager::on_chunks_updated(const CellsUpdatedEvent& event)
    {
        {
            const std::unique_lock lock{ m_generated_chunks_mutex };
            for (const v3i& chunk_pos : event.updated_cells)
                m_generated_chunks.insert(chunk_pos);
        }

        // Update lighting on all chunks surrounding updated chunks
        std::unordered_set<v3i> chunks_to_update_lighting{};
        for (const v3i& chunk_pos : event.updated_cells)
        {
            voxel_utils::for_v3i(chunk_pos - v3i{1}, chunk_pos + v3i{2},
                [&](const v3i& adj_chunk_pos)
                {
                    if (is_ready_for_lighting_update(adj_chunk_pos))
                        chunks_to_update_lighting.insert(adj_chunk_pos);
                }
            );
        }

        request_lighting_update(chunks_to_update_lighting);
    }

    void ChunkManager::on_chunks_deleted(const CellsDeletedEvent& event)
    {
        {
            const std::unique_lock lock{ m_chunk_positions_pending_lighting_update_mutex };

            for (const v2i& column_pos : event.deleted_cell_columns)
            {
                for (i32 i = 0; i < voxel_constants::vertical_chunk_count; i++)
                    m_chunk_positions_pending_lighting_update.erase(v3i{ column_pos.x, i, column_pos.y });
            }
        }

        {
            const std::unique_lock lock{ m_chunk_positions_after_lighting_update_mutex };

            for (const v2i& column_pos : event.deleted_cell_columns)
            {
                for (i32 i = 0; i < voxel_constants::vertical_chunk_count; i++)
                    m_chunk_positions_after_lighting_update.erase(v3i{ column_pos.x, i, column_pos.y });
            }
        }

        {
            const std::unique_lock lock{ m_built_chunk_lightings_mutex };

            for (const v2i& column_pos : event.deleted_cell_columns)
            {
                for (i32 i = 0; i < voxel_constants::vertical_chunk_count; i++)
                    m_built_chunk_lightings.erase(v3i{ column_pos.x, i, column_pos.y });
            }
        }

        {
            const std::unique_lock lock{ m_generated_chunks_mutex };

            for (const v2i& column_pos : event.deleted_cell_columns)
            {
                for (i32 i = 0; i < voxel_constants::vertical_chunk_count; i++)
                    m_generated_chunks.erase(v3i{ column_pos.x, i, column_pos.y });
            }
        }
    }

    namespace lighting_util
    {
        static constexpr i32 distance_from_chunk_one_axis(const i32 pos)
        {
            if (pos < 0)
                return glm::abs(pos);

            if (pos >= voxel_constants::chunk_size)
                return pos - voxel_constants::chunk_size + 1;

            return 0;
        }

        static constexpr i32 distance_from_chunk_three_axes(const v3i& pos)
        {
            return
                distance_from_chunk_one_axis(pos.x) +
                distance_from_chunk_one_axis(pos.y) +
                distance_from_chunk_one_axis(pos.z);
        }
    }

    void chunk_lighting::update_lighting(ChunkLighting& chunk_lighting, const Chunk::ViewType& chunk_view)
    {
        chunk_lighting.reset();

        constexpr v3i min_block{ -i32(voxel_constants::max_light_level) };
        constexpr v3i max_block{ voxel_constants::chunk_size + voxel_constants::max_light_level };

        const v3i center_chunk_pos = chunk_view.center_cell_pos();
        const v3i corner_chunk_pos = chunk_view.corner_cell_pos();

        ChunkLighting::ViewType lighting_view{ corner_chunk_pos, v3i{ 3 } };

        std::vector<ChunkLighting> temporary_chunk_lightings{};
        temporary_chunk_lightings.reserve(3 * 3 * 3 - 1);

        voxel_utils::for_v3i(corner_chunk_pos, corner_chunk_pos + v3i{ 3 },
            [&](const v3i& chunk_pos)
            {
                if (chunk_pos == center_chunk_pos)
                {
                    lighting_view.add_cell(chunk_pos, chunk_lighting);
                    return;
                }

                lighting_view.add_cell(chunk_pos, temporary_chunk_lightings.emplace_back());
            }
        );

        {
            std::vector<v3i> light_sources{};
            chunk_view.for_each_cell(
                [&](const Chunk& chunk, const v3i& chunk_pos)
                {
                    for (const u32 block_index : chunk.light_emitting_blocks())
                    {
                        const v3i chunk_offset = chunk_pos - chunk_view.center_cell_pos();
                        const v3i block_pos = Chunk::to_block_pos(block_index) + chunk_offset * voxel_constants::chunk_size;

                        lighting_view.set_light_level(block_pos, voxel_constants::max_light_level, ChunkLightingType::Light, EViewRelativeTo::ViewCenter);
                        light_sources.push_back(block_pos);
                    }
                }
            );

            if (!light_sources.empty())
                propagate_lighting(lighting_view, chunk_view, ChunkLightingType::Light, light_sources);
        }

        {
            std::vector<v3i> light_sources{};

            {
                ScopeTimer timer{ "UPDATE_LIGHTING::FETCH" };

                for (i32 i = -1; i <= 1; i++)
                for (i32 j = -1; j <= 1; j++)
                {
                    const auto chunk = chunk_view.get({i, 0, j}, EViewRelativeTo::ViewCenter);
                    if (!chunk)
                        continue;

                    const auto& heightmap = chunk->column_heightmap();
                    if (!heightmap)
                        continue;

                    for (i32 height_x = 0; height_x < voxel_constants::chunk_size; height_x++)
                    for (i32 height_z = 0; height_z < voxel_constants::chunk_size; height_z++)
                    {
                        v3i block_pos{
                            height_x + i * voxel_constants::chunk_size, 0,
                            height_z + j * voxel_constants::chunk_size
                        };

                        if (block_pos.x <  min_block.x || block_pos.z <  min_block.z ||
                            block_pos.x >= max_block.x || block_pos.z >= max_block.z)
                            continue;

                        const u32 height = heightmap->get_height({ height_x, height_z });
                        const i32 height_relative_to_center = i32(height) - center_chunk_pos.y * voxel_constants::chunk_size;

                        // static constexpr std::array offsets
                        // {
                        //     v2i{-1, 0 },
                        //     v2i{ 1, 0 },
                        //     v2i{ 0,-1 },
                        //     v2i{ 0, 1 },
                        // };
                        //
                        // u32 max_adj_height = 0;
                        // for (const v2i offset : offsets)
                        // {
                        //     const v2i adj_pos = offset + v2i{ height_x, height_z };
                        //     const bool is_adj_pos_valid =
                        //         adj_pos.x >= 0 && adj_pos.x < voxel_constants::chunk_size &&
                        //         adj_pos.y >= 0 && adj_pos.y < voxel_constants::chunk_size;
                        //
                        //     max_adj_height = glm::max(max_adj_height, is_adj_pos_valid ?
                        //         heightmap->get_height(adj_pos) : voxel_constants::vertical_block_count);
                        // }
                        // u32 max_adj_height_relative_to_center = i32(max_adj_height) - center_chunk_pos.y * voxel_constants::chunk_size;

                        u32 max_adj_height_relative_to_center = chunk_view.max_neighbour_height({ block_pos.x, block_pos.z }) - center_chunk_pos.y * voxel_constants::chunk_size;

                        block_pos.y = height_relative_to_center;
                        for (; block_pos.y < max_block.y; block_pos.y++)
                        {
                            if (block_pos.y < min_block.y)
                                continue;

                            lighting_view.set_light_level(block_pos, voxel_constants::max_light_level, ChunkLightingType::Sunlight, EViewRelativeTo::ViewCenter);

                            if (block_pos.y <= max_adj_height_relative_to_center)
                                light_sources.push_back(block_pos);
                        }
                    }
                }
            }

            propagate_lighting(lighting_view, chunk_view, ChunkLightingType::Sunlight, light_sources);
        }
    }

    void chunk_lighting::propagate_lighting(
        ChunkLighting::ViewType& lighting_view,
        const Chunk::ViewType& chunk_view,
        const ChunkLightingType lighting_type,
        const std::vector<v3i>& light_sources)
    {
        ScopeTimer timer{ "UPDATE_LIGHTING::PROPAGATE" };

        const auto& voxel_module = g_engine->get_module_checked<VoxelModule>();

        std::array<std::vector<v3i>, voxel_constants::max_light_level + 1> propagation_stacks{};
        propagation_stacks[voxel_constants::max_light_level] = light_sources;

        constexpr v3i min_block{ -i32(voxel_constants::max_light_level) };
        constexpr v3i max_block{ voxel_constants::chunk_size + voxel_constants::max_light_level };

        for (auto it = propagation_stacks.rbegin(); it != propagation_stacks.rend(); ++it)
        {
            for (const v3i& lit_block_pos : *it)
            {
                static constexpr std::array offsets{
                    v3i{-1, 0, 0 },
                    v3i{ 1, 0, 0 },
                    v3i{ 0,-1, 0 },
                    v3i{ 0, 1, 0 },
                    v3i{ 0, 0,-1 },
                    v3i{ 0, 0, 1 },
                };

                const u8 light_level = lighting_view
                    .get_light_level(lit_block_pos, EViewRelativeTo::ViewCenter)
                    .get(lighting_type);

                for (const v3i& offset : offsets)
                {
                    const v3i adj_block_pos = lit_block_pos + offset;

                    if (adj_block_pos.x < min_block.x || adj_block_pos.x >= max_block.x ||
                        adj_block_pos.y < min_block.y || adj_block_pos.y >= max_block.y ||
                        adj_block_pos.z < min_block.z || adj_block_pos.z >= max_block.z)
                        continue;

                    const Block adj_block = chunk_view.get_block_at(adj_block_pos, EViewRelativeTo::ViewCenter);
                    if (!voxel_module.is_transparent(adj_block.id))
                        continue;

                    const u8 adj_light_level = lighting_view
                        .get_light_level(adj_block_pos, EViewRelativeTo::ViewCenter)
                        .get(lighting_type);

                    if (adj_light_level < (light_level - 1) && light_level > 1)
                    {
                        const i32 distance_from_chunk = lighting_util::distance_from_chunk_three_axes(adj_block_pos);
                        if (light_level > distance_from_chunk)
                        {
                            lighting_view.set_light_level(adj_block_pos, light_level - 1, lighting_type, EViewRelativeTo::ViewCenter);
                            propagation_stacks[light_level - 1].push_back(adj_block_pos);
                        }
                    }
                }
            }
        }

        log::info("{}", light_sources.size());
    }
}
