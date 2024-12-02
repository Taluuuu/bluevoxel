#include "voxel/chunk_view.h"

namespace h2o
{
    ChunkView_OLD::ChunkView_OLD(const v3i& corner, const v3i& view_size)
        : m_corner(corner), m_view_size(view_size)
    {
        assert(view_size.x > 0 && view_size.y > 0 && view_size.z > 0);
        m_chunks.resize(view_size.x * view_size.y * view_size.z, nullptr);
    }

    void ChunkView_OLD::for_each_chunk(const std::function<void(Chunk&)>& function)
    {
        for (Chunk* chunk : m_chunks)
        {
            if (chunk)
                function(*chunk);
        }
    }

    void ChunkView_OLD::for_each_chunk(const std::function<void(const Chunk&)>& function) const
    {
        for (const Chunk* chunk : m_chunks)
        {
            if (chunk)
                function(*chunk);
        }
    }

    void ChunkView_OLD::for_each_block(const std::function<void(const v3i&, const Block&)>& function) const
    {
        for (const Chunk* chunk : m_chunks)
        {
            if (!chunk || chunk->is_empty())
                continue;

            const v3i chunk_corner_pos = chunk->chunk_pos() * voxel_constants::chunk_size;
            for (i32 i = 0; i < voxel_constants::chunk_size; i++)
            for (i32 j = 0; j < voxel_constants::chunk_size; j++)
            for (i32 k = 0; k < voxel_constants::chunk_size; k++)
            {
                const v3i local_block_pos{ i, j, k };
                const v3i world_block_pos = chunk_corner_pos + local_block_pos;
                if (const auto block = chunk->get_block_at(local_block_pos); block != Block::Air)
                    function(world_block_pos, block);
            }
        }
    }

    std::optional<Block> ChunkView_OLD::get_block_at(const v3i& block_pos, const ViewRelativeTo relative_to) const
    {
        if (const Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
            return chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));

        return std::nullopt;
    }

    bool ChunkView_OLD::set_block_at(const v3i& block_pos, const Block block, const ViewRelativeTo relative_to)
    {
        if (Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
        {
            chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
            return true;
        }

        return false;
    }

    std::optional<std::tuple<Block, u8>> ChunkView_OLD::get_block_and_light_level_at(
        const v3i& block_pos,
        const ViewRelativeTo relative_to) const
    {
        if (const Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
        {
            const v3i within_chunk = voxel_utils::block_pos_to_within_chunk(block_pos);
            return std::tuple {
                chunk->get_block_at(within_chunk),
                chunk->get_light_level_at(within_chunk)
            };
        }

        return std::nullopt;
    }

    Chunk* ChunkView_OLD::get_chunk_at(const v3i& relative_chunk_pos, const ViewRelativeTo relative_to)
    {
        const v3i offset = get_relative_to_chunk_pos(relative_to) - m_corner;
        const v3i local_chunk_pos = relative_chunk_pos + offset;

        if (!in_bounds(local_chunk_pos))
            return nullptr;

        return m_chunks[to_index(local_chunk_pos)];
    }

    const Chunk* ChunkView_OLD::get_chunk_at(const v3i& relative_chunk_pos, const ViewRelativeTo relative_to) const
    {
        const v3i offset = get_relative_to_chunk_pos(relative_to) - m_corner;
        const v3i local_chunk_pos = relative_chunk_pos + offset;

        if (!in_bounds(local_chunk_pos))
            return nullptr;

        return m_chunks[to_index(local_chunk_pos)];
    }

    bool ChunkView_OLD::is_generated() const
    {
        for (const auto& chunk : m_chunks)
        {
            if (!chunk || !chunk->is_generated())
                return false;
        }

        return true;
    }

    void ChunkView_OLD::add_chunk(Chunk& chunk, const std::shared_ptr<ChunkColumnData>& chunk_column)
    {
        const v3i local_chunk_pos = chunk.chunk_pos() - m_corner;
        assert(in_bounds(local_chunk_pos));

        m_chunks[to_index(local_chunk_pos)] = &chunk;
        m_chunk_columns.insert(chunk_column);
    }

    v3i ChunkView_OLD::get_relative_to_chunk_pos(const ViewRelativeTo relative_to) const
    {
        switch (relative_to)
        {
        case ViewRelativeTo::ViewCenter:
            return center_chunk_pos();
        case ViewRelativeTo::ViewCorner:
            return m_corner;
        default:
        case ViewRelativeTo::World:
            return v3i{ 0 };
        }
    }

    bool ChunkView_OLD::in_bounds(const v3i& local_chunk_pos) const
    {
        return
            local_chunk_pos.x >= 0 && local_chunk_pos.x < m_view_size.x &&
            local_chunk_pos.y >= 0 && local_chunk_pos.y < m_view_size.y &&
            local_chunk_pos.z >= 0 && local_chunk_pos.z < m_view_size.z;
    }

    size_t ChunkView_OLD::to_index(const v3i& local_chunk_pos) const
    {
        assert(in_bounds(local_chunk_pos));

        return
            local_chunk_pos.z * m_view_size.x * m_view_size.y +
            local_chunk_pos.y * m_view_size.x +
            local_chunk_pos.x;
    }
}