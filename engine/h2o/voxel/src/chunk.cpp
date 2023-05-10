#include "voxel/chunk.h"

#include "core/events.h"
#include "voxel/voxel_constants.h"
#include "voxel/chunk_mgr.h"

namespace h2o
{
    Chunk::Chunk(const ChunkMgr& chunk_mgr)
        : m_chunk_mgr(&chunk_mgr)
    {
        m_blocks.resize(voxel_constants::chunk_volume, Block::Air);
    }

    Block Chunk::get_block_at(const v3u& local_pos) const
    {
        assert(is_valid_pos(local_pos));
        return m_blocks[to_index(local_pos)];
    }

    void Chunk::set_block_at(const v3u& local_pos, Block block)
    {
        assert(is_valid_pos(local_pos));
        m_blocks[to_index(local_pos)] = block;

        // Very temporary, call the chunk updated function every time a block is changed.
        m_chunk_mgr->on_chunk_updated.broadcast({ *this });
    }

    constexpr size_t Chunk::to_index(const v3u& local_pos)
    {
        return
            local_pos.x * voxel_constants::chunk_area +
            local_pos.y * voxel_constants::chunk_size +
            local_pos.z;
    }

    constexpr bool Chunk::is_valid_pos(const v3u& local_pos)
    {
        return
            local_pos.x >= 0 && local_pos.x < voxel_constants::chunk_size &&
            local_pos.y >= 0 && local_pos.y < voxel_constants::chunk_size &&
            local_pos.z >= 0 && local_pos.z < voxel_constants::chunk_size;
    }
}