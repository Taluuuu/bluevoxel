#pragma once

#include "block.h"

#include <optional>

namespace h2o
{
    class Chunk;

    class IBlockContainer
    {
    public:

        virtual ~IBlockContainer() = default;

        /**
         * Get a chunk at the input position in chunk space
         *
         * @param chunk_pos The chunk's position in chunk space
         * @return The chunk or nullptr on failure
         */
        [[nodiscard]] virtual Chunk* get_chunk_at(const v3i& chunk_pos) = 0;
        [[nodiscard]] virtual const Chunk* get_chunk_at(const v3i& chunk_pos) const = 0;

        /**
         * Get a block in the container
         *
         * @param block_pos The block position in world space
         * @return The block or std::nullopt if block_pos is out of bounds or if the chunk is not present
         */
        [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos) const;

        /**
         * Get a block in the container
         *
         * @param block_pos The block position in world space
         * @param out_chunk The chunk in which the block was found
         * @return The block or std::nullopt if block_pos is out of bounds or if the chunk is not present
         */
        [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos, const Chunk*& out_chunk) const;
        [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos, Chunk*& out_chunk);

        /**
         * Set a block in the container
         *
         * @param block_pos The block position in world space
         * @param block The block to set
         * @return true on success, false otherwise
         */
        bool set_block_at(const v3i& block_pos, Block block);

    };
}