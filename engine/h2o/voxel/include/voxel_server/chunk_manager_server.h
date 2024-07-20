#pragma once

#include "voxel/chunk_manager_base.h"

namespace h2o
{
    class INetPeer;

    class ChunkManager_Server : public ChunkManager_Base
    {
    public:

        explicit ChunkManager_Server(INetPeer& server);

        // ChunkManager_Base interface
        bool set_block_at(const v3i& block_pos, Block block, bool replicate) override;

    private:

        INetPeer* m_server = nullptr;

    };
}