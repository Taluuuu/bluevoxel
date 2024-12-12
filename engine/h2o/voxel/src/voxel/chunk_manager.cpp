#include "voxel/chunk_manager.h"

#include "core/engine.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    ChunkManager::ChunkManager()
    {
        // cells_updated_event<Chunk>().add_listener(m_chunks_updated_handle,
        //     [this](const CellsUpdatedEvent& event)
        //     {
        //         for (const v3i& chunk_pos : event.updated_cells)
        //         {
        //
        //         }
        //     }
        // );
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

    void chunk_lighting::update_lighting(
        ChunkLighting& chunk_lighting,
        const Chunk::ViewType& chunk_view)
    {
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

        std::queue<v3i> propagation_queue{};
        voxel_utils::for_v3i(
            -v3i{voxel_constants::max_light_level},
            v3i{voxel_constants::max_light_level + voxel_constants::chunk_size},
            [&](const v3i& block_pos)
            {
                if (chunk_view.get_block_at(block_pos, EViewRelativeTo::ViewCenter) == 5)
                {
                    lighting_view.set_light_level(block_pos, voxel_constants::max_light_level, EViewRelativeTo::ViewCenter);
                    propagation_queue.push(block_pos);
                }
            }
        );

        while (!propagation_queue.empty())
        {
            const v3i lit_block_pos = propagation_queue.front();
            propagation_queue.pop();

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
                // if (!Chunk::is_valid_pos(adj_block_pos))
                //     continue;

                const Block block = chunk_view.get_block_at(adj_block_pos, EViewRelativeTo::ViewCenter);
                if (!voxel_module.is_transparent(block.id))
                    continue;

                const u8 adj_light_level = lighting_view.get_light_level(adj_block_pos, EViewRelativeTo::ViewCenter);
                if (adj_light_level < (light_level - 1) && light_level > 1)
                {
                    lighting_view.set_light_level(adj_block_pos, light_level - 1, EViewRelativeTo::ViewCenter);
                    propagation_queue.push(adj_block_pos);
                }
            }
        }
    }
}
