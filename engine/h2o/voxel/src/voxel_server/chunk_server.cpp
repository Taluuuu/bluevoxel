#include "voxel_server/chunk_server.h"

#include "core/engine.h"
#include "core/log.h"
#include "networking/server.h"
#include "scene/scene.h"
#include "voxel/chunk_generators/chunk_generator_flat.h"
#include "voxel/chunk_generators/chunk_generator_terrain.h"
#include "voxel/voxel_net_messages.h"
#include "voxel/voxel_pack.h"

namespace h2o
{
    ChunkServer::ChunkServer(
        const SceneSystemInitializer& system_initializer,
        INetPeer& server)
        : SceneSystem(system_initializer)
        , m_server(&server)
        , m_chunk_region_mgr(*this)
    {
        // Bind messages
        server.handle_message<net_msg::ChunkFetchRequest>(m_received_chunk_request_handle,
            [&](const PeerID client_id, const net_msg::ChunkFetchRequest& chunk_fetch_request)
            {
                log::info("Received {} chunk fetch requests.", chunk_fetch_request.requested_chunks.size());
                on_received_chunk_fetch_requests(client_id, chunk_fetch_request);
            }
        );

        server.handle_message<net_msg::BlockPlaceRequest>(m_received_block_place_request_handle,
            [&](const PeerID client_id, const net_msg::BlockPlaceRequest& block_place_request)
            {
                log::info("Received block place request.");
                on_received_block_place_request(client_id, block_place_request);
            }
        );

        if (const auto voxel_pack = g_engine->get_module_checked<VoxelModule>().voxel_pack())
            m_chunk_generator = voxel_pack->chunk_generator();

        set_tick_phases(TickPhase::Update);
    }

    void ChunkServer::regenerate()
    {
        m_chunk_mgr.remove_all_chunk_columns([](v2i){ return true; });
        m_chunk_region_mgr.clear();
    }

    void ChunkServer::update(f32 delta_time)
    {
        erase_if(m_chunks_pending_send,
            [&](const std::pair<v2i, PeerID>& chunk_pos_peer_pair)
            {
                const auto [chunk_pos, client_id] = chunk_pos_peer_pair;

                bool was_chunk_sent = false;
                m_chunk_mgr.view_chunk_column(chunk_pos,
                    [&](const ChunkColumnView& chunk_column)
                    {
                        if (chunk_column.is_generated())
                        {
                            send_chunk_column(chunk_column, { client_id });
                            was_chunk_sent = true;
                        }
                        else
                        {
                            m_chunk_region_mgr.generate_regions_for_chunk(chunk_pos);
                        }
                    }
                );

                return was_chunk_sent;
            }
        );
    }

    void ChunkServer::on_received_chunk_fetch_requests(
        PeerID client_id,
        const net_msg::ChunkFetchRequest& chunk_fetch_request)
    {
        for (v2i requested_chunk : chunk_fetch_request.requested_chunks)
            m_chunks_pending_send.emplace_back(requested_chunk, client_id);
    }

    void ChunkServer::on_received_block_place_request(
        PeerID request_sender,
        const net_msg::BlockPlaceRequest& block_place_request)
    {
        if (!m_chunk_mgr.set_block_at(block_place_request.block_pos, block_place_request.placed_block))
            return;

        for (PeerID client_id : m_server->peers())
        {
            if (client_id != request_sender)
                m_server->send_message(client_id, block_place_request);
        }
    }

    void ChunkServer::send_chunk_column(const ChunkColumnView& chunk_col, const std::set<PeerID>& client_ids) const
    {
        // Send chunk column to requesting clients
        std::vector<CompressedChunk> compressed_chunks{};
        compressed_chunks.reserve(voxel_constants::vertical_chunk_count);

        chunk_col.for_each_chunk(
            [&](const Chunk& chunk)
            {
                compressed_chunks.emplace_back(chunk);
            }
        );

        const v2i chunk_column_pos{
            chunk_col.corner_chunk_pos().x,
            chunk_col.corner_chunk_pos().z };

        for (PeerID client : client_ids)
        {
            m_server->send_message(client,
                net_msg::ChunkFetchResult{
                    compressed_chunks, chunk_column_pos
                }
            );
        }
    }
}