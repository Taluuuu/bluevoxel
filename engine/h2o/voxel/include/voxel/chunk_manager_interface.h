#pragma once

#include "core/types.h"
#include "voxel/chunk_container_interface.h"
#include "voxel_constants.h"

#include <memory>

namespace h2o
{
    // class ChunkColumn;
    // class ChunkView;

    // class ChunkManager_Base
    // {
    // public:
    //
    //     virtual ~ChunkManager_Base() = default;
    //
    //     [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos) const;
    //     virtual bool set_block_at(const v3i& block_pos, Block block, bool replicate);
    //
    //     void fetch_chunk(const v3i& chunk_pos, const std::function<void(Chunk*)>& function);
    //     void fetch_chunk(const v3i& chunk_pos, const std::function<void(const Chunk*)>& function) const;
    //
    // private:
    //
    //     [[nodiscard]] std::shared_ptr<ChunkColumn> create_chunk_column(v2i chunk_column_pos) const;
    //     [[nodiscard]] std::shared_ptr<ChunkColumn> find_chunk_column(v2i chunk_column_pos) const;
    //     [[nodiscard]] std::shared_ptr<ChunkColumn> find_or_create_chunk_column(v2i chunk_column_pos);
    //
    // private:
    //
    //     std::unordered_map<v2i, std::shared_ptr<ChunkColumn>> m_loaded_chunks{};
    //     mutable std::shared_mutex m_loaded_chunks_mutex{};
    //
    // };

    // class IChunkManager_OLD : public IChunkContainer
    // {
    // public:
    //
//     ~IChunkManager_OLD() override = default;
    //
    //     /**
    //      * Fetch the chunk column at the input position. The function
    //      *
    //      * @param chunk_column_pos The chunk column's position
    //      * @param lock_chunks True if the chunks within this chunk column should be locked, or
    //      *                    just the chunk column will be accessed.
    //      * @param function A lambda taking a chunk column pointer and a bool that is true if
    //      *                 the chunk column was just created. The pointer will be null if the
    //      *                 chunk column does not exist.
    //      */
    //     virtual void fetch_chunk_column(v2i chunk_column_pos, bool lock_chunks, const std::function<void(ChunkColumn*)>& function) = 0;
    //     virtual void fetch_chunk_column(v2i chunk_column_pos, bool lock_chunks, const std::function<void(const ChunkColumn*)>& function) const = 0;
    //
    //     /**
    //      * Fetch a chunk column, and create it if it did not already exist.
    //      *
    //      * @param chunk_column_pos The chunk column's position
    //      * @param lock_chunks True if the chunks within this chunk column should be locked, or
    //      *                    just the chunk column will be accessed.
    //      * @param function A lambda taking a chunk column reference and a bool that is true if
    //      *                 the chunk column was just created
    //      */
    //     virtual void fetch_or_create_chunk_column(v2i chunk_column_pos, bool lock_chunks, const std::function<void(ChunkColumn&)>& function) = 0;
    //
    //     virtual void fetch_chunk_region(const std::vector<v3i>& chunk_positions, const std::function<void(const ChunkRegion_OLD&)>& function) = 0;
    //     // min, max are inclusive
    //     virtual void fetch_chunk_region(v3i min, v3i max, const std::function<void(ChunkRegion_OLD&)>& function) = 0;
    //     void fetch_chunk_region(v2i min, v2i max, const std::function<void(ChunkRegion_OLD&)>& function);
    //
    //     /**
    //      * Erase all chunk columns that are more than range chunks away from a position
    //      *
    //      * @param positions Positions in chunk coords
    //      * @param range Range around positions in chunk coords
    //      */
    //     virtual void erase_far_chunks(const std::vector<v2i>& positions, i32 range) = 0;
    //
    //     [[nodiscard]] virtual bool is_chunk_column_generated(v2i chunk_column_pos) const = 0;
    //
    //     // IChunkContainer interface
    //     void fetch_chunk(const v3i& chunk_pos, const std::function<void(Chunk*)>& function) override;
    //     void fetch_chunk(const v3i& chunk_pos, const std::function<void(const Chunk*)>& function) const override;
    //
    // };
}