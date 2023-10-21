#pragma once

#include "core/events.h"
#include "voxel/chunk_manager_base.h"
#include "voxel/voxel_net_messages.h"

namespace h2o
{
    class Client;

    class ChunkManager_Client : public ChunkManager_Base
    {
    public:

        explicit ChunkManager_Client(Client& client);
        ~ChunkManager_Client() override = default;

        bool set_block_at(const v3i& block_pos, Block block, bool replicate);

    public:

        Event<net_msg::BlockPlaceRequest> on_placed_block;

    private:

        Client* m_client = nullptr;

    };
}