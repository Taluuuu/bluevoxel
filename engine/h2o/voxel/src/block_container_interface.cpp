#include "voxel/block_container_interface.h"

#include "voxel/chunk.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    std::optional<Block> IBlockContainer::get_block_at(const v3i& block_pos) const
    {
        if (const Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos)))
            return chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));

        return std::nullopt;
    }

    std::optional<Block> IBlockContainer::get_block_at(const v3i& block_pos, const Chunk*& out_chunk) const
    {
        out_chunk = nullptr;
        if (const Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos)))
        {
            out_chunk = chunk;
            return chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));
        }

        return std::nullopt;
    }

    std::optional<Block> IBlockContainer::get_block_at(const v3i& block_pos, Chunk*& out_chunk)
    {
        out_chunk = nullptr;
        if (Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos)))
        {
            out_chunk = chunk;
            return chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));
        }

        return std::nullopt;
    }

    bool IBlockContainer::set_block_at(const v3i& block_pos, Block block)
    {
        if (Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos)))
        {
            chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
            return true;
        }

        return false;
    }
}