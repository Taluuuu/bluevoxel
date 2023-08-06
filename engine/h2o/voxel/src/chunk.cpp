#include "voxel/chunk.h"

#include "voxel/voxel_constants.h"
#include "voxel/chunk_system.h"

namespace h2o
{
    static size_t to_index(const v3i& local_pos)
    {
        return
            local_pos.x * voxel_constants::chunk_area +
            local_pos.y * voxel_constants::chunk_size +
            local_pos.z;
    }

    void Chunk::init(const v3i& chunk_pos)
    {
        m_chunk_pos = chunk_pos;

        m_blocks.resize(voxel_constants::chunk_volume, Block::Air);
    }

    Block Chunk::get_block_at(const v3i& local_pos) const
    {
        assert(is_valid_pos(local_pos));
        return m_blocks[to_index(local_pos)];
    }

    void Chunk::set_block_at(const v3i& local_pos, Block block)
    {
        assert(is_valid_pos(local_pos));

        // TODO: Check if the block is valid

        m_blocks[to_index(local_pos)] = block;

        // TODO: Add way for is_empty to be reset to true
        if (block != Block::Air)
            m_is_empty = false;
    }
}