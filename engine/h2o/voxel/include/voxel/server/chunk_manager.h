#pragma once

#include "core/handle_types.h"
#include "core/types.h"

#include "glm/gtx/hash.hpp"
#include <mutex>
#include <unordered_map>

namespace h2o
{
    class ChunkColumn;
    class ChunkManager;

    class ChunkRegion
    {
    public:

        ChunkRegion(v2i center, ChunkManager& chunk_manager);

    private:

        std::array<WeakHandle<ChunkColumn>, 9> m_chunks;

    };

    class ChunkManager
    {
    public:

        [[nodiscard]] WeakHandle<ChunkColumn> fetch_chunk_at(v2i chunk_pos) const;
        [[nodiscard]] WeakHandle<ChunkColumn> fetch_or_create_chunk_at(v2i chunk_pos);

    protected:

        [[nodiscard]] OwningHandle<ChunkColumn> create_chunk_column(v2i chunk_pos) const;

    private:

        mutable std::mutex m_mutex;
        std::unordered_map<v2i, OwningHandle<ChunkColumn>> m_loaded_chunks{};

    };
}