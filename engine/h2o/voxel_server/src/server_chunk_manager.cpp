#include "voxel_server/server_chunk_manager.h"

#include "voxel/chunk_column.h"

namespace h2o
{
    std::shared_ptr<ChunkColumn> ServerChunkManager::fetch_chunk_at(v2i chunk_pos) const
    {
        std::lock_guard lock { m_mutex };

        const auto it = m_loaded_chunks.find(chunk_pos);
        if (it == m_loaded_chunks.end())
            return nullptr;

        return { it->second };
    }

    std::shared_ptr<ChunkColumn> ServerChunkManager::fetch_or_create_chunk_at(v2i chunk_pos)
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

    std::shared_ptr<ChunkColumn> ServerChunkManager::create_chunk_column(v2i chunk_pos) const
    {
        return std::make_shared<ChunkColumn>(chunk_pos);
    }
}