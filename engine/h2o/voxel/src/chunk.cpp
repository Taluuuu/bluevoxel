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

    void Chunk::init()
    {
        m_blocks.resize(voxel_constants::chunk_volume, Block::Air);
    }

    Block Chunk::get_block_at(const v3i& local_pos) const
    {
        assert(is_valid_pos(local_pos));
        assert(is_initialized());

        return m_blocks[to_index(local_pos)];
    }

    Block* Chunk::get_block_ptr_at(const v3i& local_pos)
    {
        assert(is_valid_pos(local_pos));
        assert(is_initialized());

        return &m_blocks[to_index(local_pos)];
    }

    void Chunk::set_block_at(const v3i& local_pos, Block block)
    {
        assert(is_valid_pos(local_pos));
        assert(is_initialized());

        // TODO: Check if the block is valid

        m_blocks[to_index(local_pos)] = block;

        if (block == Block::Air)
        {
            // Check if chunk is now empty
            // Whoops, this is really fucking slow right now because of world gen code...
            // This check should really be done after that and each time a block is placed.
//            m_is_empty = true;
//            for (const Block& b : m_blocks)
//            {
//                if (b != Block::Air)
//                {
//                    m_is_empty = false;
//                    break;
//                }
//            }
        }
        else
        {
            m_is_empty = false;
        }
    }

    ChunkColumn::ChunkColumn(v2i chunk_col_pos)
        : m_chunk_col_pos(chunk_col_pos)
    {
        i32 y = 0;
        for (auto& chunk : m_chunks)
            chunk.m_chunk_pos = { chunk_col_pos.x, y++, chunk_col_pos.y };
    }

    void ChunkColumn::init()
    {
        assert(!m_is_initialized);
        for (auto& chunk : m_chunks)
            chunk.init();

        m_is_initialized = true;
    }

    void ChunkColumn::increment_generation_stage()
    {
        m_generation_stage++;
    }

    void ChunkColumn::finish_generation()
    {
        m_generation_stage = voxel_constants::max_generation_stage;
    }

    Chunk* ChunkWeakHandle::chunk() const
    {
        if (!chunk_column)
            return nullptr;

        return &(*chunk_column)[height];
    }

    bool ChunkWeakHandle::operator==(const ChunkWeakHandle& other) const
    {
        return
            chunk_column == other.chunk_column &&
            height       == other.height;
    }
}