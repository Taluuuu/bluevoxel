#pragma once

#include "voxel/chunk_manager_base.h"

namespace h2o
{
    class ChunkRegion;
    class INetPeer;

    class ChunkManager_Server : public ChunkManager_Base
    {
    public:

        explicit ChunkManager_Server(INetPeer& server);

        // ChunkManager_Base interface
        bool set_block_at(const v3i& block_pos, Block block, bool replicate) override;

        void generate_region(v2i region_pos);

    private:

        INetPeer* m_server = nullptr;

        std::unordered_map<v2i, std::shared_ptr<ChunkRegion>> m_chunk_regions{};
        mutable std::shared_mutex m_chunk_regions_mutex{};

    };
}