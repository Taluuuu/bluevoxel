#include "voxel/chunk_manager.h"

namespace h2o
{
    ChunkRegion::ChunkRegion(v2i center, IChunkManager& chunk_manager)
    {
        size_t index = 0;
        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        {
            auto chunk_col = chunk_manager.fetch_or_create_chunk_at(
                center + v2i{ i, j });

            assert(chunk_col);

            m_chunks[index++] = chunk_col;
        }
    }

    void ChunkRegion::for_each_chunk_column(const std::function<void(const std::shared_ptr<ChunkColumn>&)>& fun) const
    {
        for (const auto& chunk_col : m_chunks)
        {
            assert(chunk_col);
            fun(chunk_col);
        }
    }

    const std::shared_ptr<ChunkColumn>& ChunkRegion::center_chunk() const
    {
        return m_chunks[4]; // :)
    }
}