#include "voxel_server/chunk_manager_server.h"

#include <voxel/voxel_net_messages.h>

#include "networking/net_peer.h"

namespace h2o
{
    ChunkManager_Server::ChunkManager_Server(INetPeer& server)
        : m_server(&server)
    {}

    bool ChunkManager_Server::set_block_at(const v3i& block_pos, Block block, bool replicate)
    {
        if (ChunkManager_Base::set_block_at(block_pos, block, replicate))
        {
            if (replicate)
            {
                // TODO: Investigate having a remote peer interface where we could just
                //       type peer.send_message(...);
                const net_msg::BlockPlaceRequest block_place_request{ block, block_pos };

                for (const PeerID peer : m_server->peers())
                    m_server->send_message(peer, block_place_request);
            }

            return true;
        }

        return false;
    }
}
