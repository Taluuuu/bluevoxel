#pragma once

#include "core/types.h"
#include "voxel/block_container_interface.h"

namespace h2o
{
    class ChunkColumn;
    class ChunkRegion;

    class IChunkManager : public IBlockContainer
    {
    public:

        ~IChunkManager() override = default;

        /**
         * Fetch the chunk column at the input position. The function
         *
         * @param chunk_column_pos The chunk column's position
         * @param function A lambda taking a chunk column pointer and a bool that is true if
         *                 the chunk column was just created. The pointer will be null if the
         *                 chunk column does not exist.
         */
        virtual void fetch_chunk_column(v2i chunk_column_pos, const std::function<void(ChunkColumn*)>& function) = 0;
        virtual void fetch_chunk_column(v2i chunk_column_pos, const std::function<void(const ChunkColumn*)>& function) const = 0;

        /**
         * Fetch a chunk column, and create it if it did not already exist.
         *
         * @param chunk_column_pos The chunk column's position
         * @param function A lambda taking a chunk column reference and a bool that is true if
         *                 the chunk column was just created
         */
        virtual void fetch_or_create_chunk_column(v2i chunk_column_pos, const std::function<void(ChunkColumn&, bool)>& function) = 0;

        virtual void fetch_chunk_region(v2i chunk_region_center, const std::function<void(const ChunkRegion&)>& function) = 0;

        /**
         * Erase all chunk columns that are more than range chunks away from a position
         *
         * @param positions Positions in chunk coords
         * @param range Range around positions in chunk coords
         */
        virtual void erase_far_chunks(const std::vector<v2i>& positions, i32 range) = 0;

        // IBlockContainer interface
        void fetch_chunk_at(const v3i& chunk_pos, const std::function<void(Chunk* chunk)>& function) override;
        void fetch_chunk_at(const v3i& chunk_pos, const std::function<void(const Chunk* chunk)>& function) const override;

    };
}