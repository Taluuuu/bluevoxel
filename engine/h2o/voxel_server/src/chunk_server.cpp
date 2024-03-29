#include "voxel_server/chunk_server.h"

#include "core/engine.h"
#include "core/log.h"
#include "networking/message_ids.h"
#include "networking/server.h"
#include "voxel/voxel_net_messages.h"
#include "voxel/voxel_transport.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    ChunkServer::ChunkServer(
        const SceneSystemInitializer& system_initializer,
        const std::shared_ptr<IVoxelTransport>& voxel_transport)
        : SceneSystem(system_initializer)
        , m_voxel_transport(voxel_transport)
        , m_world_generator(m_chunk_mgr)
    {
        assert(voxel_transport);

        voxel_transport->on_chunk_column_requested().add_listener(m_received_chunk_request_handle,
            [this](const auto& event)
            {
                on_received_chunk_fetch_requests(event);
            }
        );

        voxel_transport->on_block_place().add_listener(m_block_place_handle,
            [this](const auto& event)
            {
                on_received_block_place_request(event);
            }
        );

        set_tick_phases(TickPhase::Update);
    }

    void ChunkServer::update(f32 delta_time)
    {

    }

    void ChunkServer::on_received_chunk_fetch_requests(
        PeerID client_id,
        const std::vector<v2i>& requested_chunks)
    {
        for (v2i requested_chunk : requested_chunks)
        {
            m_world_generator.request_chunk_column(requested_chunk,
                [this, client_id](const ChunkColumn& chunk_column)
                {
                    send_chunk_column(chunk_column, { client_id });
                }
            );
        }
    }

    void ChunkServer::on_received_block_place_request(
        PeerID request_sender,
        const net_msg::BlockPlaceRequest& block_place_request)
    {
        if (!m_chunk_mgr.set_block_at(block_place_request.block_pos, block_place_request.placed_block))
            return;

        for (PeerID client_id : m_server->peers())
        {
            if (client_id == request_sender)
                continue;

            m_server->send_message(client_id, block_place_request);
        }
    }

    void ChunkServer::send_chunk_column(const ChunkColumn& chunk_col, const std::set<PeerID>& client_ids) const
    {
        // Send chunk column to requesting clients
        std::vector<CompressedChunk> compressed_chunks{};
        compressed_chunks.reserve(voxel_constants::vertical_chunk_count);

        for (size_t i = 0; i < voxel_constants::vertical_chunk_count; i++)
            compressed_chunks.push_back(chunk_col[i].compress());

        for (PeerID client: client_ids)
        {
            m_server->send_message(client,
                net_msg::ChunkFetchResult
                {
                    compressed_chunks,
                    chunk_col.chunk_column_pos()
                }
            );
        }
    }
}