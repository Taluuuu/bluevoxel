#include "voxel/chunk_region.h"

#include "voxel/chunk_column.h"

namespace h2o
{
    ChunkRegion::ChunkRegion(v2i center)
        : m_center(center)
    {
        for (auto& chunk_col : m_chunks)
            chunk_col = nullptr;
    }

    ChunkColumn& ChunkRegion::center_chunk() const
    {
        auto chunk_column = m_chunks[to_index({0, 0})];
        assert(chunk_column);
        return *chunk_column;
    }

    void ChunkRegion::for_each_chunk_column(const std::function<void(ChunkColumn&)>& function) const
    {
        for (const auto& chunk_col : m_chunks)
        {
            assert(chunk_col);
            function(*chunk_col);
        }
    }

    void ChunkRegion::add_chunk_column(const std::shared_ptr<ChunkColumn>& chunk_column)
    {
        assert(chunk_column);

        const v2i local_pos = chunk_column->chunk_column_pos() - m_center;
        assert(in_range(local_pos));

        m_chunks[to_index(local_pos)] = chunk_column;
    }

    bool ChunkRegion::in_range(v2i local_chunk_column_pos) const
    {
        return
            local_chunk_column_pos.x >= -1 && local_chunk_column_pos.x <= 1 &&
            local_chunk_column_pos.y >= -1 && local_chunk_column_pos.y <= 1;
    }

    size_t ChunkRegion::to_index(v2i local_chunk_column_pos) const
    {
        const v2i relative_to_corner = local_chunk_column_pos + v2i{ 1, 1 };
        assert(relative_to_corner.x >= 0 && relative_to_corner.y >= 0);

        return relative_to_corner.x * 3 + relative_to_corner.y;
    }
}