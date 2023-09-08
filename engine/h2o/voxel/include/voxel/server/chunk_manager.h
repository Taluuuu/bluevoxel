#pragma once

#include "core/handle_types.h"
#include "core/types.h"

#include <glm/gtx/hash.hpp>
#include <memory>
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

        [[nodiscard]] const std::shared_ptr<ChunkColumn>& center_chunk() const;

        void for_each_chunk_column(const std::function<void(const std::shared_ptr<ChunkColumn>&)>& fun) const;

    private:

        std::array< std::shared_ptr<ChunkColumn>, 9 > m_chunks;

    };

    class ChunkManager
    {
    public:

        [[nodiscard]] std::shared_ptr<ChunkColumn> fetch_chunk_at(v2i chunk_pos) const;
        [[nodiscard]] std::shared_ptr<ChunkColumn> fetch_or_create_chunk_at(v2i chunk_pos);

    protected:

        [[nodiscard]] std::shared_ptr<ChunkColumn> create_chunk_column(v2i chunk_pos) const;

    private:

        mutable std::mutex m_mutex;
        std::unordered_map< v2i, std::shared_ptr<ChunkColumn> > m_loaded_chunks{};

    };
}