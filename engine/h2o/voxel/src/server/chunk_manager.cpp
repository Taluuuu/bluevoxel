#include "voxel/server/chunk_manager.h"

#include "voxel/chunk_column.h"

namespace h2o
{
    ChunkRegion::ChunkRegion(v2i center, ChunkManager& chunk_manager)
    {
        size_t index = 0;
        for (i32 i = -1; i <= 1; i++)
        for (i32 j = -1; j <= 1; j++)
        {
            m_chunks[index++] = chunk_manager.fetch_or_create_chunk_at(
                center + v2i{ i, j });
        }
    }

    WeakHandle<ChunkColumn> ChunkManager::fetch_chunk_at(v2i chunk_pos) const
    {
        std::lock_guard lock { m_mutex };
        const auto it = m_loaded_chunks.find(chunk_pos);
        if (it == m_loaded_chunks.end())
            return nullptr;

        return { it->second };
    }

    WeakHandle<ChunkColumn> ChunkManager::fetch_or_create_chunk_at(v2i chunk_pos)
    {
        if (const auto chunk_col = fetch_chunk_at(chunk_pos))
            return chunk_col; // Chunk column already exists

        // Need to create a new chunk column.
        // The chunk generation will come later on.
        auto chunk_col = create_chunk_column(chunk_pos);
        const WeakHandle<ChunkColumn> weak_chunk_col = chunk_col;
        assert(chunk_col);

        std::lock_guard lock { m_mutex };
        m_loaded_chunks[chunk_pos] = std::move(chunk_col);

        return weak_chunk_col;
    }

    OwningHandle<ChunkColumn> ChunkManager::create_chunk_column(v2i chunk_pos) const
    {
        return oup::make_observable_unique<ChunkColumn>(chunk_pos);
    }
}