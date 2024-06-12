#include "voxel/chunk_region.h"

#include "voxel/chunk_column.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    ChunkRegion_OLD::ChunkRegion_OLD(const v3i& min, const v3i& size)
        : m_chunks(size.x * size.y * size.z, nullptr)
        , m_min(min)
        , m_size(size)
    { }

    std::optional<Block> ChunkRegion_OLD::get_block_at(const v3i& block_pos, const v3i& relative_to_chunk_pos) const
    {
        if (const Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos), relative_to_chunk_pos))
            return chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));

        return std::nullopt;
    }

    bool ChunkRegion_OLD::set_block_at(const v3i& block_pos, Block block, const v3i& relative_to_chunk_pos)
    {
        if (Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos), relative_to_chunk_pos))
        {
            chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
            return true;
        }

        return false;
    }

    Chunk* ChunkRegion_OLD::get_chunk_at(const v3i& relative_chunk_pos, const v3i& relative_to)
    {
        const v3i offset = relative_to - m_min;
        const v3i local_chunk_pos = relative_chunk_pos + offset;

        if (!in_range(local_chunk_pos))
            return nullptr;

        return m_chunks[to_index(local_chunk_pos)];
    }

    const Chunk* ChunkRegion_OLD::get_chunk_at(const v3i& relative_chunk_pos, const v3i& relative_to) const
    {
        const v3i offset = relative_to - m_min;
        const v3i local_chunk_pos = relative_chunk_pos + offset;

        if (!in_range(local_chunk_pos))
            return nullptr;

        return m_chunks[to_index(local_chunk_pos)];
    }

    v3i ChunkRegion_OLD::center_chunk_pos() const
    {
        return m_min + m_size / 2;
    }

    void ChunkRegion_OLD::add_chunk(Chunk& chunk)
    {
        const v3i local_chunk_pos = chunk.chunk_pos() - m_min;
        assert(in_range(local_chunk_pos));

        m_chunks[to_index(local_chunk_pos)] = &chunk;
    }

    void ChunkRegion_OLD::lock_chunks(bool exclusive)
    {
        for (Chunk* chunk : m_chunks)
        {
            if (!chunk)
                continue;

            if (exclusive)
            {
                chunk->mutex().lock();
            }
            else
            {
                chunk->mutex().lock_shared();
            }
        }
    }

    void ChunkRegion_OLD::unlock_chunks(bool exclusive)
    {
        for (Chunk* chunk : m_chunks)
        {
            if (!chunk)
                continue;

            if (exclusive)
            {
                chunk->mutex().unlock();
            }
            else
            {
                chunk->mutex().unlock_shared();
            }
        }
    }

    bool ChunkRegion_OLD::in_range(const v3i& local_chunk_pos) const
    {
        return
            local_chunk_pos.x >= 0 && local_chunk_pos.x < m_size.x &&
            local_chunk_pos.y >= 0 && local_chunk_pos.y < m_size.y &&
            local_chunk_pos.z >= 0 && local_chunk_pos.z < m_size.z;
    }

    size_t ChunkRegion_OLD::to_index(const v3i& local_chunk_pos) const
    {
        assert(in_range(local_chunk_pos));

        return
            local_chunk_pos.z * m_size.x * m_size.y +
            local_chunk_pos.y * m_size.x +
            local_chunk_pos.x;
    }
}