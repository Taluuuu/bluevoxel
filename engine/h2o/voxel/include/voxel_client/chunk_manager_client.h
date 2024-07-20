#pragma once

#include "core/events.h"
#include "voxel/chunk_manager_base.h"
#include "voxel/voxel_net_messages.h"

namespace h2o
{
    class INetPeer;

    class ChunkManager_Client : public ChunkManager_Base
    {
    public:

        explicit ChunkManager_Client(INetPeer& client);

        // ChunkManager_Base interface
        bool set_block_at(const v3i& block_pos, Block block, bool replicate) override;

    public:

        Event<net_msg::BlockPlaceRequest> on_placed_block;

    private:

        INetPeer* m_client = nullptr;

    };
}