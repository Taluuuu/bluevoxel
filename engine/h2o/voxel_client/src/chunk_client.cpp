#include "voxel_client/chunk_client.h"

#include "core/engine.h"
#include "rendering/camera.h"
#include "rendering/pipeline.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture_array.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_net_messages.h"
#include "voxel/voxel_utils.h"
#include "voxel_rendering/voxel_rendering_module.h"

#include <glm/gtx/norm.hpp>

namespace h2o
{
    ChunkClient::ChunkClient(
        const SceneSystemInitializer& system_initializer,
        Client& client)
        : SceneSystem(system_initializer)
        , m_client(&client)
        , m_chunk_mgr(client)
    {
        set_tick_phases(TickPhase::Update | TickPhase::Render);

        m_rendering_module       = &g_engine->get_module_checked<RenderingModule>();
        m_voxel_rendering_module = &g_engine->get_module_checked<VoxelRenderingModule>();

        m_client->on_connected_to_server.add_listener(m_on_connected_handle,
            [&](const Client::OnConnectedEvent& on_connected_event)
            {
                m_refresh_chunk_requests = true;
            }
        );

        m_client->handle_message<net_msg::ChunkFetchResult>(m_on_fetched_chunk_handle,
            [&](PeerID client_id, const net_msg::ChunkFetchResult& chunk_fetch_result)
            {
                auto& [compressed_chunks, chunk_pos] = chunk_fetch_result;

                m_num_received_chunks++;

                if (!is_in_range(chunk_pos))
                    return;

                if (compressed_chunks.size() != voxel_constants::vertical_chunk_count)
                    return;

                // Decompressing a chunk is slow. Run it on a thread.
                g_engine->thread_pool().queue_job(
                    [&, compressed_chunks, chunk_pos]()
                    {
                        m_chunk_mgr.fetch_or_create_chunk_column(chunk_pos, true,
                            [&](ChunkColumn& chunk_column, bool was_just_created)
                            {
                                // Decompress chunks
                                for (size_t i = 0; i < voxel_constants::vertical_chunk_count; i++)
                                {
                                    auto& chunk = chunk_column[i];
                                    chunk.decompress(compressed_chunks[i]);

                                    if (!chunk.is_empty())
                                        m_chunk_meshing_queue.enqueue(chunk.chunk_pos());
                                }

                                chunk_column.finish_generation();
                            }
                        );
                    }
                );
            }
        );

        m_client->handle_message<net_msg::BlockPlaceRequest>(m_on_received_block_place_request,
            [&](PeerID client_id, const net_msg::BlockPlaceRequest& block_place_request)
            {
                // TODO: It seems like the block placed event is called twice
                if (m_chunk_mgr.set_block_at(block_place_request.block_pos, block_place_request.placed_block, false))
                    m_chunk_meshing_queue.enqueue(voxel_utils::block_to_chunk_pos(block_place_request.block_pos));
            }
        );

        m_chunk_mgr.on_placed_block.add_listener(m_on_block_placed,
            [&](const net_msg::BlockPlaceRequest& block_place_request)
            {
                m_chunk_meshing_queue.enqueue(voxel_utils::block_to_chunk_pos(block_place_request.block_pos));
            }
        );
    }

    void ChunkClient::update(f32 delta_time)
    {
//        log::info("{} / {}", m_num_received_chunks, m_num_requested_chunks);

        if (!m_client->is_connected())
            return;

        auto player = m_scene->get_actor_by_tag(ActorTag::LocalPlayer);
        if (!player)
            return;

        m_chunk_meshing_queue.set_player_actor(player);

        const v3& player_pos = player->transform.position;
        const v3i player_chunk_pos = voxel_utils::world_to_chunk_pos(player_pos);
        const v2i player_chunk_col_pos { player_chunk_pos.x, player_chunk_pos.z };

        if (player_chunk_col_pos != m_previous_player_chunk_col_pos || m_refresh_chunk_requests)
        {
            m_previous_player_chunk_col_pos = player_chunk_col_pos;

            request_chunk_loads();
            trim_far_chunks();
        }

        m_refresh_chunk_requests = false;

        build_chunk_meshes(5, player_pos);
    }

    void ChunkClient::render()
    {
        auto render_system = m_scene->get_system<RenderingSystem>();
        if (!render_system)
            return;

        const auto& camera = render_system->main_camera();
        if (!camera)
            return;

        const auto& pipeline = m_voxel_rendering_module->pipeline();
        const auto& block_textures = m_voxel_rendering_module->block_textures();

        auto& renderer = m_rendering_module->renderer();
        renderer.bind_pipeline(pipeline);

        // TODO: Avoid calculating proj_view matrix here.
        const m4 proj_view = camera->calc_proj_view();
        pipeline->set_uniform_mat4(0, proj_view);

        block_textures->bind(0);
        pipeline->set_uniform_int(2, 0);
        pipeline->set_uniform_vec3(3, glm::normalize(light_dir));
        pipeline->set_uniform_vec3(4, light_color);
        pipeline->set_uniform_float(5, ambient_strength);

        m_chunk_mesh_pool.for_each_chunk_mesh(
            [&](const ChunkMesh& chunk_mesh)
            {
                if (!chunk_mesh.is_empty())
                {
                    pipeline->set_uniform_ivec3(1, chunk_mesh.chunk_pos());
                    renderer.draw(chunk_mesh.vertex_array(), chunk_mesh.vertex_count());
                }
            }
        );
    }

    bool ChunkClient::is_in_range(v2i chunk_pos) const
    {
        const i32 valid_dist = m_view_distance + m_stay_loaded_distance;
        const v2i min = m_previous_player_chunk_col_pos - v2i { valid_dist, valid_dist };
        const v2i max = m_previous_player_chunk_col_pos + v2i { valid_dist, valid_dist };

        return
            chunk_pos.x >= min.x && chunk_pos.x <= max.x &&
            chunk_pos.y >= min.y && chunk_pos.y <= max.y;
    }

    void ChunkClient::request_chunk_loads()
    {
        net_msg::ChunkFetchRequest chunk_fetch_request{};

        for (i32 i = -m_view_distance; i <= m_view_distance; i++)
        for (i32 j = -m_view_distance; j <= m_view_distance; j++)
        {
            const v2i chunk_column_pos {
                m_previous_player_chunk_col_pos.x + i,
                m_previous_player_chunk_col_pos.y + j };

            // TODO: Add a for each chunk column in range function to the chunk mgr
            //       to avoid locking the mutex every time
            m_chunk_mgr.fetch_chunk_column(chunk_column_pos, false,
                [&](const ChunkColumn* chunk_column)
                {
                    if (!chunk_column)
                        chunk_fetch_request.requested_chunks.push_back(chunk_column_pos);
                }
            );
        }

        if (!chunk_fetch_request.requested_chunks.empty())
            m_client->send_message(0, chunk_fetch_request);

        m_num_requested_chunks += i32(chunk_fetch_request.requested_chunks.size());
    }

    void ChunkClient::trim_far_chunks()
    {
        m_chunk_mgr.erase_far_chunks({ m_previous_player_chunk_col_pos }, m_view_distance);
    }

    void ChunkClient::build_chunk_meshes(i32 max_chunk_meshes, const v3& player_pos)
    {
        for (i32 i = 0; i < max_chunk_meshes; i++)
        {
            if (const auto chunk_to_mesh = m_chunk_meshing_queue.dequeue_first(
                [&](const v3i& chunk_pos)
                {
                    const v2i chunk_column_pos{chunk_pos.x, chunk_pos.z};

                    return
                        m_chunk_mgr.is_chunk_column_generated(
                            voxel::to_vec2(voxel::Direction::XNeg) + chunk_column_pos) &&
                            m_chunk_mgr.is_chunk_column_generated(
                                voxel::to_vec2(voxel::Direction::XPos) + chunk_column_pos) &&
                            m_chunk_mgr.is_chunk_column_generated(
                                voxel::to_vec2(voxel::Direction::ZNeg) + chunk_column_pos) &&
                            m_chunk_mgr.is_chunk_column_generated(
                                voxel::to_vec2(voxel::Direction::ZPos) + chunk_column_pos);
                }))
            {
                build_chunk_mesh_at(*chunk_to_mesh);
            }
            else
            {
                break;
            }
        }
    }

    void ChunkClient::build_chunk_mesh_at(const v3i& chunk_pos)
    {
        std::vector<v3i> region_chunk_positions;
        region_chunk_positions.reserve(7);

        region_chunk_positions.push_back(chunk_pos);
        magic_enum::enum_for_each<voxel::Direction::Type>(
            [&](voxel::Direction::Type dir)
            {
                const v3i offset = voxel::to_vec3(dir);
                region_chunk_positions.push_back(chunk_pos + offset);
            }
        );

        m_chunk_mgr.fetch_chunk_region(region_chunk_positions,
            [&](const ChunkRegion& chunk_region)
            {
                const Chunk* chunk = chunk_region.get_chunk_at(chunk_pos);
                if (!chunk || chunk->is_empty()) // TODO: Could chunk->is_empty() here cause a problem when destroying the last block of a chunk ?
                    return;

                m_chunk_mesh_pool.fetch_or_create_chunk_mesh(chunk_pos,
                    [&](ChunkMesh& chunk_mesh)
                    {
                        chunk_mesh.update(chunk_region);
                    }
                );
            }
        );
    }
}