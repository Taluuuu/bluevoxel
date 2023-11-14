#pragma once

#include "chunk_manager_interface.h"

#include <glm/gtx/hash.hpp>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace h2o
{
    // Thread-safe IChunkManager implementation using std::unordered_map.
    class ChunkManager_Base : public IChunkManager
    {
    public:

        ~ChunkManager_Base() override = default;

        // IChunkManager interface
        void fetch_chunk_column(v2i chunk_column_pos, bool lock_chunks, const std::function<void(ChunkColumn*)>& function) override;
        void fetch_chunk_column(v2i chunk_column_pos, bool lock_chunks, const std::function<void(const ChunkColumn*)>& function) const override;
        void fetch_or_create_chunk_column(v2i chunk_column_pos, bool lock_chunks, const std::function<void(ChunkColumn&, bool)>& function) override;
        void fetch_chunk_region(const std::vector<v3i>& chunk_positions, const std::function<void(const ChunkRegion&)>& function) override;
        void fetch_chunk_region(v3i min, v3i max, const std::function<void(ChunkRegion&)>& function) override;
        void erase_far_chunks(const std::vector<v2i>& positions, i32 range) override;

    private:

        [[nodiscard]] std::shared_ptr<ChunkColumn> create_chunk_column(v2i chunk_column_pos) const;
        [[nodiscard]] std::shared_ptr<ChunkColumn> find_chunk_column(v2i chunk_column_pos) const;
        [[nodiscard]] std::shared_ptr<ChunkColumn> find_or_create_chunk_column(v2i chunk_column_pos);
        [[nodiscard]] std::shared_ptr<ChunkColumn> find_or_create_chunk_column(v2i chunk_column_pos, bool& out_was_just_created);

    private:

        std::unordered_map<v2i, std::shared_ptr<ChunkColumn>> m_loaded_chunks{};
        mutable std::shared_mutex m_loaded_chunks_mutex{};

    };
}