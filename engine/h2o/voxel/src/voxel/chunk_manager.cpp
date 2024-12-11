#include "voxel/chunk_manager.h"

#include "core/engine.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    ChunkManager::ChunkManager()
    {
        cells_updated_event<Chunk>().add_listener(m_chunks_updated_handle,
            [this](const CellsUpdatedEvent& event)
            {
                for (const v3i& chunk_pos : event.updated_cells)
                {
                //     view_mut<ChunkLighting>(chunk_pos - v3i{1}, v3i{3},
                //         [&](View<ChunkLighting>& lighting_view)
                //         {
                                view<Chunk>(chunk_pos - v3i{1}, v3i{3},
                                    [&](const View<Chunk>& chunk_view)
                                    {
                                        // chunk_lighting::update_lighting(lighting_view, chunk_view);
                                    }
                                );
                //         }
                //     );
                }
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

    std::optional<Block> voxel::get_block_at(
        const View<Chunk>& view,
        const v3i& block_pos,
        const EViewRelativeTo relative_to)
    {
        if (const Chunk* chunk = view.get(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
            return chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));

        return std::nullopt;
    }

    bool voxel::set_block_at(
        View<Chunk>& view,
        const v3i& block_pos,
        const Block block,
        const EViewRelativeTo relative_to)
    {
        if (Chunk* chunk = view.get(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
        {
            chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
            return true;
        }

        return false;
    }

    void voxel::for_each_block(
        const View<Chunk>& view,
        const std::function<void(const v3i&, const Block&)>& function)
    {
        view.for_each_cell(
            [&](const Chunk& chunk, const v3i&)
            {
                if (chunk.is_empty())
                    return;

                const v3i chunk_corner_pos = chunk.chunk_pos() * voxel_constants::chunk_size;
                for (i32 i = 0; i < voxel_constants::chunk_size; i++)
                for (i32 j = 0; j < voxel_constants::chunk_size; j++)
                for (i32 k = 0; k < voxel_constants::chunk_size; k++)
                {
                    const v3i local_block_pos{ i, j, k };
                    const v3i world_block_pos = chunk_corner_pos + local_block_pos;
                    if (const auto block = chunk.get_block_at(local_block_pos); block != Block::Air)
                        function(world_block_pos, block);
                }
            }
        );
    }

    bool voxel::is_generated(const View<Chunk>& view)
    {
        return !view.any_matches([](const Chunk* chunk) { return !chunk || !chunk->is_generated(); });
    }

    void chunk_lighting::update_lighting(
        View<ChunkLighting>& lighting_view,
        const View<Chunk>& chunk_view)
    {
        // TODO: Maybe figure some better place for this
        const VoxelModule& voxel_module = g_engine->get_module_checked<VoxelModule>();

        const v3i center_chunk_pos = chunk_view.center_cell_pos();

        // for ()
        // chunk_lighting::set_light_level(lighting_view, )
        // lighting_view.get()

        // lighting_view.for_each_cell(
        //     [&](ChunkLighting& chunk_lighting, const v3i& chunk_lighting_pos)
        //     {
        //         const v3i chunk_lighting_offset = chunk_lighting_pos - lighting_view.center_cell_pos();
        //
        //     }
        // );

        // std::ranges::fill(m_light_levels, LightLevel{0, 2});
        // std::queue<v3i> propagation_queue{};
        // for (i32 i = 0; i < voxel_constants::chunk_size; i++)
        // for (i32 j = 0; j < voxel_constants::chunk_size; j++)
        // for (i32 k = 0; k < voxel_constants::chunk_size; k++)
        // {
        //     const v3i block_pos{ i, j, k };
        //     if (get_block_function(block_pos) == 5)
        //     {
        //         set_light_level(block_pos, voxel_constants::max_light_level);
        //         propagation_queue.push(block_pos);
        //     }
        // }
        //
        // while (!propagation_queue.empty())
        // {
        //     const v3i lit_block_pos = propagation_queue.front();
        //     propagation_queue.pop();
        //
        //     static constexpr std::array offsets{
        //         v3i{-1, 0, 0 },
        //         v3i{ 1, 0, 0 },
        //         v3i{ 0,-1, 0 },
        //         v3i{ 0, 1, 0 },
        //         v3i{ 0, 0,-1 },
        //         v3i{ 0, 0, 1 },
        //     };
        //
        //     const u8 light_level = get_light_level(lit_block_pos);
        //     for (const v3i& offset : offsets)
        //     {
        //         const v3i adj_block_pos = lit_block_pos + offset;
        //         if (!Chunk::is_valid_pos(adj_block_pos))
        //             continue;
        //
        //         const Block block = get_block_function(adj_block_pos);
        //         if (!voxel_module.is_transparent(block.id))
        //             continue;
        //
        //         const u8 adj_light_level = get_light_level(adj_block_pos);
        //         if (adj_light_level < (light_level - 1) && light_level > 1)
        //         {
        //             set_light_level(adj_block_pos, light_level - 1);
        //             propagation_queue.push(adj_block_pos);
        //         }
        //     }
        // }
    }

    void chunk_lighting::set_light_level(
        View<ChunkLighting>& view,
        const v3i& block_pos,
        const u8 light_level,
        const EViewRelativeTo relative_to)
    {
        if (ChunkLighting* chunk_lighting = view.get(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
            chunk_lighting->set_light_level(voxel_utils::block_pos_to_within_chunk(block_pos), light_level);
    }

    u8 chunk_lighting::get_light_level(View<ChunkLighting>& view, const v3i& block_pos, const EViewRelativeTo relative_to)
    {
        if (const ChunkLighting* chunk_lighting = view.get(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
            return chunk_lighting->get_light_level(voxel_utils::block_pos_to_within_chunk(block_pos));

        return 0;
    }
}
