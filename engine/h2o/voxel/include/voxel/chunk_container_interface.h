#pragma once

#include "block.h"

#include <functional>
#include <optional>

namespace h2o
{
    class Chunk;

    class IChunkContainer
    {
    public:

        virtual ~IChunkContainer() = default;

        /**
         * Get a block in the container.
         *
         * @param block_pos The block position in world space
         * @return The block or std::nullopt if block_pos is out of bounds or if the chunk is not present
         */
        [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos) const;

        /**
         * Set a block in the container.
         *
         * @param block_pos The block position in world space
         * @param block The block to set
         * @return true on success, false otherwise
         */
        virtual bool set_block_at(const v3i& block_pos, Block block);

        /**
         * Fetch the chunk at the input position and executes the lambda. The lambda will be run
         * with nullptr if the chunk is not loaded.
         *
         * @param chunk_pos The chunk's position
         * @param function The function to run with the chunk
         */
        virtual void fetch_chunk(const v3i& chunk_pos, const std::function<void(Chunk* chunk)>& function) = 0;
        virtual void fetch_chunk(const v3i& chunk_pos, const std::function<void(const Chunk* chunk)>& function) const = 0;

    };
}