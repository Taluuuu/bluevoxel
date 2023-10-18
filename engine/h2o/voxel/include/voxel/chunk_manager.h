#pragma once

#include "core/handle_types.h"
#include "core/types.h"

#include "glm/gtx/hash.hpp"
#include <memory>
#include <mutex>
#include <unordered_map>

namespace h2o
{
    class ChunkColumn;

    class IChunkManager
    {
    public:

        virtual ~IChunkManager() = default;

        /**
         * Fetch a chunk column at the input position, returns nullptr if there is none.
         *
         * @param chunk_pos The chunk column's position
         * @return The chunk column or nullptr if there was none
         */
        [[nodiscard]] virtual std::shared_ptr<ChunkColumn> fetch_chunk_at(v2i chunk_pos) const = 0;

        /**
         * Fetch a chunk column at the input position, creates it if it did not already exist.
         *
         * @param chunk_pos The chunk column's position
         * @return The chunk column, always valid.
         */
        [[nodiscard]] virtual std::shared_ptr<ChunkColumn> fetch_or_create_chunk_at(v2i chunk_pos) = 0;

    };

    class ChunkRegion
    {
    public:

        ChunkRegion(v2i center, IChunkManager& chunk_manager);

        [[nodiscard]] const std::shared_ptr<ChunkColumn>& center_chunk() const;

        void for_each_chunk_column(const std::function<void(const std::shared_ptr<ChunkColumn>&)>& fun) const;

    private:

        std::array< std::shared_ptr<ChunkColumn>, 9 > m_chunks;

    };
}