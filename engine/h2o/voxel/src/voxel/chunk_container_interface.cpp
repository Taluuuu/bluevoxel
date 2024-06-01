#include "voxel/chunk_container_interface.h"

#include "voxel/chunk.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    std::optional<Block> IChunkContainer::get_block_at(const v3i& block_pos) const
    {
        std::optional<Block> block = std::nullopt;
        fetch_chunk(voxel_utils::block_to_chunk_pos(block_pos),
            [&](const Chunk* chunk)
            {
                if (chunk)
                    block = chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));
            }
        );

        return block;
    }

    bool IChunkContainer::set_block_at(const v3i& block_pos, Block block)
    {
        bool success = false;
        fetch_chunk(voxel_utils::block_to_chunk_pos(block_pos),
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
}