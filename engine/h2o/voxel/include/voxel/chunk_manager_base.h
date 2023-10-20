#pragma once

#include "chunk_manager_interface.h"

#include <glm/gtx/hash.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace h2o
{
    // IChunkManager implementation using std::unordered_map.
    class ChunkManager_Base : public IChunkManager
    {
    public:

        ~ChunkManager_Base() override = default;

        // IChunkManager interface
        void fetch_chunk_column(v2i chunk_column_pos, const std::function<void(ChunkColumn*)>& function) override;
        void fetch_chunk_column(v2i chunk_column_pos, const std::function<void(const ChunkColumn*)>& function) const override;
        void fetch_or_create_chunk_column(v2i chunk_column_pos, const std::function<void(ChunkColumn&, bool)>& function) override;
        void fetch_chunk_region(v2i chunk_region_center, const std::function<void(const ChunkRegion&)>& function) override;
        void erase_far_chunks(const std::vector<v2i>& positions, i32 range) override;

    private:

        [[nodiscard]] std::shared_ptr<ChunkColumn> create_chunk_column(v2i chunk_column_pos) const;
        [[nodiscard]] ChunkColumn* find_chunk_column(v2i chunk_column_pos) const;

    private:

        std::unordered_map<v2i, std::shared_ptr<ChunkColumn>> m_loaded_chunks{};
        mutable std::mutex m_loaded_chunks_mutex{};

    };
}