#include "voxel_server/chunk_server.h"

#include "core/engine.h"
#include "core/log.h"
#include "networking/message_ids.h"
#include "networking/server.h"
#include "voxel/voxel_net_messages.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    ChunkServer::ChunkServer(const SceneSystemInitializer& system_initializer, Server& server)
        : SceneSystem(system_initializer)
        , m_server(&server)
        , m_world_generator(m_chunk_mgr)
    {
        // Bind messages
        server.handle_message<net_msg::ChunkFetchRequest>(m_received_chunk_request_handle,
            [&](PeerID client_id, const net_msg::ChunkFetchRequest& chunk_fetch_request)
            {
                log::info("Received {} chunk fetch requests.", chunk_fetch_request.requested_chunks.size());
                on_received_chunk_fetch_requests(client_id, chunk_fetch_request);
            }
        );

        server.handle_message<net_msg::BlockPlaceRequest>(m_received_block_place_request_handle,
            [&](PeerID client_id, const net_msg::BlockPlaceRequest& block_place_request)
            {
                log::info("Received block place request.");
                on_received_block_place_request(client_id, block_place_request);
            }
        );

        set_tick_phases(TickPhase::Update);
    }

    void ChunkServer::update(f32 delta_time)
    {
        log::info("{}", m_num_sent_chunks);
    }

    void ChunkServer::on_received_chunk_fetch_requests(
        PeerID client_id,
        const net_msg::ChunkFetchRequest& chunk_fetch_request)
    {
        for (v2i requested_chunk : chunk_fetch_request.requested_chunks)
        {
            m_world_generator.request_chunk_column(requested_chunk,
                [&, client_id](const ChunkColumn& chunk_column)
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

        m_num_sent_chunks++;
    }
}