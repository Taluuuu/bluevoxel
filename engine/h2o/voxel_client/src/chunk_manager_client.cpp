#include "voxel_client/chunk_manager_client.h"

#include "networking/net_peer.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_net_messages.h"

namespace h2o
{
    ChunkManager_Client::ChunkManager_Client(INetPeer& client)
        : m_client(&client)
    {}

    bool ChunkManager_Client::set_block_at(const v3i& block_pos, Block block, bool replicate)
    {
        if (IChunkContainer::set_block_at(block_pos, block))
        {
            if (replicate)
                m_client->send_message(0, net_msg::BlockPlaceRequest { block, block_pos });

            on_placed_block.broadcast({ block, block_pos });

            return true;
        }

        return false;
    }
}