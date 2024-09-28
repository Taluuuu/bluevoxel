#include "voxel_client/chunk_client.h"

#include "core/engine.h"
#include "networking/net_peer.h"
#include "rendering/camera.h"
#include "rendering/pipeline.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture_array.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "voxel/compressed_chunk.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_net_messages.h"
#include "voxel/voxel_utils.h"

#include <glm/gtx/norm.hpp>
#include <magic_enum_all.hpp>
#include <voxel/chunk_view.h>

namespace h2o
{
    ChunkClient::ChunkClient(
        const SceneSystemInitializer& system_initializer,
        INetPeer& client)
        : SceneSystem(system_initializer)
        , m_voxel_world_renderer(*this, m_chunk_mgr)
        , m_voxel_bounds(v2i{}, 8)
        , m_client(&client)
    {
        set_tick_phases(TickPhase::Update);

        m_rendering_module = &g_engine->get_module_checked<RenderingModule>();
        m_voxel_module     = &g_engine->get_module_checked<VoxelModule>();
        m_refresh_chunk_requests = true;

        m_client->handle_message<net_msg::ChunkFetchResult>(m_on_fetched_chunk_handle,
            [&](PeerID client_id, const net_msg::ChunkFetchResult& chunk_fetch_result)
            {
                auto& [compressed_chunks, chunk_column_pos] = chunk_fetch_result;

                if (!m_voxel_bounds.in_bounds(chunk_column_pos))
                    return;

                if (compressed_chunks.size() != voxel_constants::vertical_chunk_count)
                    return;

                ++m_num_chunk_columns_pending_decompress;

                // Decompressing a chunk is slow. Run it on a thread.
                v2 player_pos_2d{ m_player_pos.x, m_player_pos.z };
                g_engine->thread_pool().queue_job(glm::distance(player_pos_2d, voxel_utils::chunk_to_world_pos(chunk_column_pos)),
                    [this, compressed_chunks, chunk_column_pos]()
                    {
                        // TODO: A vector of compressed chunks is always a chunk column, so the class
                        //       should be CompressedChunkColumn instead so I don't have to fetch the
                        //       column at every iteration
                        for (const auto& compressed_chunk : compressed_chunks)
                        {
                            const v3i chunk_pos = compressed_chunk.chunk_pos();
                            m_chunk_mgr.fetch_or_create_chunk_mut(chunk_pos,
                                [&](Chunk* chunk)
                                {
                                    assert(chunk != nullptr);
                                    compressed_chunk.decompress(*chunk);
                                }
                            );
                        }

                        --m_num_chunk_columns_pending_decompress;
                    }
                );
            }
        );

        m_client->handle_message<net_msg::BlockPlaceRequest>(m_on_received_block_place_request,
            [this](PeerID client_id, const net_msg::BlockPlaceRequest& block_place_request)
            {
                m_chunk_mgr.set_block_at(block_place_request.block_pos, block_place_request.placed_block);
            }
        );
    }

    void ChunkClient::set_block_at(const v3i& block_pos, Block block)
    {
        if (m_chunk_mgr.set_block_at(block_pos, block))
            m_client->send_message(0, net_msg::BlockPlaceRequest(block, block_pos));
    }

    void ChunkClient::update(f32 delta_time)
    {
        if (!m_client->is_connected())
            return;

        auto player = m_scene->get_actor_by_tag(ActorTag::LocalPlayer);
        if (!player)
            return;

        m_player_pos = player->transform.position;
        const v3i player_chunk_pos = voxel_utils::world_to_chunk_pos(m_player_pos);
        const v2i player_chunk_col_pos { player_chunk_pos.x, player_chunk_pos.z };
        m_voxel_bounds.set_bounds_center(player_chunk_col_pos);

        m_voxel_world_renderer.player_pos = m_player_pos;

        if (player_chunk_col_pos != m_previous_player_chunk_col_pos || m_refresh_chunk_requests)
        {
            m_previous_player_chunk_col_pos = player_chunk_col_pos;

            request_chunk_loads();
        }

        m_refresh_chunk_requests = false;

        g_engine->debug_infos().update_debug_statistic(
            "voxels", "chunk columns pending decompress", m_num_chunk_columns_pending_decompress);

        m_chunk_mgr.broadcast_events();
    }

    void ChunkClient::request_chunk_loads()
    {
        net_msg::ChunkFetchRequest chunk_fetch_request{};

        m_voxel_bounds.for_each_pos_in_bounds(
            [&](v2i chunk_column_pos)
            {
                m_chunk_mgr.view_chunk_column(chunk_column_pos,
                    [&](const ChunkColumnView& chunk_column)
                    {
                        // TODO: This can probably request the same chunks multiple times
                        if (!chunk_column.is_generated())
                            chunk_fetch_request.requested_chunks.push_back(chunk_column_pos);
                    }
                );
            }
        );

        if (!chunk_fetch_request.requested_chunks.empty())
            m_client->send_message(0, chunk_fetch_request);
    }
}
