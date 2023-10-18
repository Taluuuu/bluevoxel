#pragma once

#include "voxel/chunk_manager.h"

namespace h2o
{
    class ServerChunkManager : public IChunkManager
    {
    public:

        ~ServerChunkManager() override = default;

        // IChunkManager interface
        [[nodiscard]] std::shared_ptr<ChunkColumn> fetch_chunk_at(v2i chunk_pos) const override;
        [[nodiscard]] std::shared_ptr<ChunkColumn> fetch_or_create_chunk_at(v2i chunk_pos) override;

    protected:

        [[nodiscard]] std::shared_ptr<ChunkColumn> create_chunk_column(v2i chunk_pos) const;

    private:

        mutable std::mutex m_mutex;
        std::unordered_map< v2i, std::shared_ptr<ChunkColumn> > m_loaded_chunks{};

    };
}