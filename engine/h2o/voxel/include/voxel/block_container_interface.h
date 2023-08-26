#pragma once

#include "block.h"
#include "chunk_weak_handle.h"

#include <optional>

namespace h2o
{
    class IBlockContainer
    {
    public:

        virtual ~IBlockContainer() = default;

        /**
         * Get a block in the container
         *
         * @param block_pos The block position in world space
         * @return The block or std::nullopt if block_pos is out of bounds or if the chunk is not present
         */
        [[nodiscard]] virtual std::optional<Block> get_block_at(const v3i& block_pos) const = 0;

        /**
         * Get a block in the container
         *
         * @param block_pos The block position in world space
         * @param out_chunk The chunk in which the block was found
         * @return The block or std::nullopt if block_pos is out of bounds or if the chunk is not present
         */
        [[nodiscard]] virtual std::optional<Block> get_block_at(const v3i& block_pos, ChunkWeakHandle& out_chunk) const = 0;

        /**
         * Set a block in the container
         *
         * @param block_pos The block position in world space
         * @param block The block to set
         * @return true on success, false otherwise
         */
        virtual bool set_block_at(const v3i& block_pos, Block block) const = 0;

    };
}