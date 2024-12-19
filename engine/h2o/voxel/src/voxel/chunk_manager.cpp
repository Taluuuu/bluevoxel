#include "voxel/chunk_manager.h"

#include "core/engine.h"
#include "core/profiling/scope_timer.h"
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
        fetch_mut<Chunk>(voxel_utils::block_to_chunk_pos(block_pos),
            [&](Chunk* chunk)
            {
                if (chunk)
                {
                    chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
                    success = true;
                }
            }
        );

        return success;
    }

    bool ChunkManager::is_chunk_column_generated(const v2i chunk_column_pos)
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

    void ChunkManager::broadcast_events()
    {
        Grid3D::broadcast_events();

        // Manually broadcast chunk lighting update event
        {
            const std::unique_lock lock{ m_chunk_positions_after_lighting_update_mutex };

            cells_updated_event<ChunkLighting>().broadcast(
                CellsUpdatedEvent{ m_chunk_positions_after_lighting_update });

            m_chunk_positions_after_lighting_update.clear();
        }
    }

    void ChunkManager::on_chunks_updated(const CellsUpdatedEvent& event)
    {
        // Update lighting on all chunks surrounding updated chunks
        std::unordered_set<v3i> chunks_to_update_lighting{};
        for (const v3i& chunk_pos : event.updated_cells)
        {
            voxel_utils::for_v3i(chunk_pos - v3i{1}, chunk_pos + v3i{2},
                [&](const v3i& adj_chunk_pos)
                {
                    chunks_to_update_lighting.insert(adj_chunk_pos);
                }
            );
        }

        bool should_queue_job = false;
        {
            const std::unique_lock lock{ m_chunk_positions_pending_lighting_update_mutex };
            should_queue_job =
                m_chunk_positions_pending_lighting_update.empty() &&
                !chunks_to_update_lighting.empty();

            m_chunk_positions_pending_lighting_update.insert(
                chunks_to_update_lighting.begin(),
                chunks_to_update_lighting.end());
        }

        if (should_queue_job)
        {
            g_engine->thread_pool().queue_job(0.0f,
                [this]
                {
                    std::unordered_set<v3i> chunk_positions_to_update_lighting{};
                    {
                        const std::unique_lock lock{ m_chunk_positions_pending_lighting_update_mutex };
                        chunk_positions_to_update_lighting =
                            std::move(m_chunk_positions_pending_lighting_update);
                    }

                    for (const v3i& chunk_pos : chunk_positions_to_update_lighting)
                    {
                        view<Chunk>(chunk_pos - v3i{1}, v3i{3},
                            [&](const Chunk::ViewType& chunk_view)
                            {
                                fetch_mut<ChunkLighting>(chunk_pos,
                                    [&](ChunkLighting* lighting)
                                    {
                                        if (lighting)
                                            chunk_lighting::update_lighting(*lighting, chunk_view);
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
                            chunk_positions_to_update_lighting.begin(),
                            chunk_positions_to_update_lighting.end());
                    }
                }
            );
        }
    }

    void chunk_lighting::update_lighting(
        ChunkLighting& chunk_lighting,
        const Chunk::ViewType& chunk_view)
    {
        ScopeTimer scope_timer("Lighting Update");

        // TODO: Maybe figure some better place for this
        const VoxelModule& voxel_module = g_engine->get_module_checked<VoxelModule>();

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

        chunk_lighting.reset();

        std::stack<v3i> propagation_stack{};
        chunk_view.for_each_cell(
            [&](const Chunk& chunk, const v3i& chunk_pos)
            {
                for (const u32 block_index : chunk.light_emitting_blocks())
                {
                    const v3i chunk_offset = chunk_pos - chunk_view.center_cell_pos();
                    const v3i block_pos = Chunk::to_block_pos(block_index) + chunk_offset * voxel_constants::chunk_size;

                    lighting_view.set_light_level(block_pos, voxel_constants::max_light_level, EViewRelativeTo::ViewCenter);
                    propagation_stack.push(block_pos);
                }
            }
        );

        while (!propagation_stack.empty())
        {
            const v3i lit_block_pos = propagation_stack.top();
            propagation_stack.pop();

            static constexpr std::array offsets{
                v3i{-1, 0, 0 },
                v3i{ 1, 0, 0 },
                v3i{ 0,-1, 0 },
                v3i{ 0, 1, 0 },
                v3i{ 0, 0,-1 },
                v3i{ 0, 0, 1 },
            };

            const u8 light_level = lighting_view.get_light_level(lit_block_pos, EViewRelativeTo::ViewCenter);
            for (const v3i& offset : offsets)
            {
                const v3i adj_block_pos = lit_block_pos + offset;

                const Block block = chunk_view.get_block_at(adj_block_pos, EViewRelativeTo::ViewCenter);
                if (!voxel_module.is_transparent(block.id))
                    continue;

                const u8 adj_light_level = lighting_view.get_light_level(adj_block_pos, EViewRelativeTo::ViewCenter);
                if (adj_light_level < (light_level - 1) && light_level > 1)
                {
                    lighting_view.set_light_level(adj_block_pos, light_level - 1, EViewRelativeTo::ViewCenter);
                    propagation_stack.push(adj_block_pos);
                }
            }
        }
    }
}
