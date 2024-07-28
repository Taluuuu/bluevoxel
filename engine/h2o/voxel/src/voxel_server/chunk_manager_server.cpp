#include "voxel_server/chunk_manager_server.h"

#include "core/engine.h"
#include "networking/net_peer.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_net_messages.h"

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

    void ChunkManager_Server::generate_region(v2i region_pos)
    {
        g_engine->thread_pool().queue_job(0.0f, [this, region_pos]
            {
                const std::unique_lock lock{ m_chunk_regions_mutex };
                if (m_chunk_regions.contains(region_pos))
                    return;

                auto& chunk_region = m_chunk_regions[region_pos];
                chunk_region = std::make_shared<ChunkRegion>(region_pos);

                // view_or_create<{
                //     voxel_constants::chunk_region_size,
                //     voxel_constants::vertical_chunk_count,
                //     voxel_constants::chunk_region_size }>(region_pos);
            }
        );
    }
}
