#include "voxel_client/chunk_manager_client.h"

#include "networking/client.h"
#include "voxel/chunk_column.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_net_messages.h"

namespace h2o
{
    ChunkManager_Client::ChunkManager_Client(Client& client)
        : m_client(&client)
    {}

    bool ChunkManager_Client::set_block_at(const v3i& block_pos, Block block)
    {
        if (IBlockContainer::set_block_at(block_pos, block))
        {
            m_client->send_message(0, net_msg::BlockPlaceRequest { block, block_pos });
            return true;
        }

        return false;
    }
}