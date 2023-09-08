#include "voxel_server/chunk_manager.h"

#include "voxel/chunk_column.h"

namespace h2o
{
    ChunkRegion::ChunkRegion(v2i center, ChunkManager& chunk_manager)
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

    std::shared_ptr<ChunkColumn> ChunkManager::fetch_chunk_at(v2i chunk_pos) const
    {
        std::lock_guard lock { m_mutex };

        const auto it = m_loaded_chunks.find(chunk_pos);
        if (it == m_loaded_chunks.end())
            return nullptr;

        return { it->second };
    }

    std::shared_ptr<ChunkColumn> ChunkManager::fetch_or_create_chunk_at(v2i chunk_pos)
    {
        if (const auto chunk_col = fetch_chunk_at(chunk_pos))
            return chunk_col; // Chunk column already exists

        // Need to create a new chunk column.
        // The chunk generation will come later on.
        auto chunk_col = create_chunk_column(chunk_pos);
        assert(chunk_col);

        std::lock_guard lock { m_mutex };
        m_loaded_chunks[chunk_pos] = chunk_col;

        return chunk_col;
    }

    std::shared_ptr<ChunkColumn> ChunkManager::create_chunk_column(v2i chunk_pos) const
    {
        return std::make_shared<ChunkColumn>(chunk_pos);
    }
}