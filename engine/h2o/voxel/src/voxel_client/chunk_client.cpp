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
        , m_voxel_bounds(v2i{}, 16)
        , m_client(&client)
    {
        set_tick_phases(TickPhase::Update | TickPhase::Render);

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

                // Decompressing a chunk is slow. Run it on a thread.
                g_engine->thread_pool().queue_job(10000.0f,
                    [this, compressed_chunks, chunk_column_pos]()
                    {
                        // TODO: A vector of compressed chunks is always a chunk column, so the class
                        //       should be CompressedChunkColumn instead so I don't have to fetch the
                        //       column at every iteration
                        for (const auto& compressed_chunk : compressed_chunks)
                        {
                            const v3i chunk_pos = compressed_chunk.chunk_pos();
                            m_chunk_mgr.fetch_or_create_chunk(chunk_pos,
                                [&](Chunk* chunk)
                                {
                                    assert(chunk != nullptr);
                                    compressed_chunk.decompress(*chunk);
                                    m_chunk_meshing_queue.enqueue(chunk_pos);
                                }
                            );
                        }
                    }
                );
            }
        );

        m_client->handle_message<net_msg::BlockPlaceRequest>(m_on_received_block_place_request,
            [this](PeerID client_id, const net_msg::BlockPlaceRequest& block_place_request)
            {
                // TODO: It seems like the block placed event is called twice
                if (m_chunk_mgr.set_block_at(block_place_request.block_pos, block_place_request.placed_block))
                    m_chunk_meshing_queue.enqueue(voxel_utils::block_to_chunk_pos(block_place_request.block_pos));
            }
        );

//        m_chunk_mgr.on_placed_block.add_listener(m_on_block_placed,
//            [this](const net_msg::BlockPlaceRequest& block_place_request)
//            {
//                m_chunk_meshing_queue.enqueue(voxel_utils::block_to_chunk_pos(block_place_request.block_pos));
//            }
//        );
    }

    void ChunkClient::update(f32 delta_time)
    {
        if (!m_client->is_connected())
            return;

        auto player = m_scene->get_actor_by_tag(ActorTag::LocalPlayer);
        if (!player)
            return;

        const v3& player_pos = player->transform.position;
        const v3i player_chunk_pos = voxel_utils::world_to_chunk_pos(player_pos);
        const v2i player_chunk_col_pos { player_chunk_pos.x, player_chunk_pos.z };
        m_voxel_bounds.set_bounds_center(player_chunk_col_pos);

        if (player_chunk_col_pos != m_previous_player_chunk_col_pos || m_refresh_chunk_requests)
        {
            m_previous_player_chunk_col_pos = player_chunk_col_pos;

            request_chunk_loads();
            trim_far_chunks();
        }

        m_refresh_chunk_requests = false;

        m_chunk_mesh_pool.update_meshes(m_voxel_bounds);

        m_chunk_meshing_queue.set_player_actor(player);
        while (auto chunk_pos = m_chunk_meshing_queue.dequeue_first(
            [&](const v3i& chunk_pos) -> bool
            {
                // const v2i chunk_column_pos { chunk_pos.x, chunk_pos.z };
                // return
                //     m_chunk_mgr.is_chunk_column_generated(voxel::to_vec2(voxel::Direction::XNeg) + chunk_column_pos) &&
                //     m_chunk_mgr.is_chunk_column_generated(voxel::to_vec2(voxel::Direction::XPos) + chunk_column_pos) &&
                //     m_chunk_mgr.is_chunk_column_generated(voxel::to_vec2(voxel::Direction::ZNeg) + chunk_column_pos) &&
                //     m_chunk_mgr.is_chunk_column_generated(voxel::to_vec2(voxel::Direction::ZPos) + chunk_column_pos);
                return true;
            }))
        {
            rebuild_chunk_mesh(*chunk_pos);
        }
    }

    void ChunkClient::render()
    {
        auto render_system = m_scene->get_system<RenderingSystem>();
        if (!render_system)
            return;

        const auto& pipeline = m_voxel_module->pipeline();
        const auto& block_textures = m_voxel_module->block_textures();

        auto& renderer = m_rendering_module->renderer();
        renderer.bind_pipeline(pipeline);
        pipeline->set_uniform_mat4(0, renderer.proj_view_matrix());

        block_textures->bind(0);
        pipeline->set_uniform_int(2, 0);
        pipeline->set_uniform_vec3(3, glm::normalize(light_dir));
        pipeline->set_uniform_vec3(4, light_color);
        pipeline->set_uniform_float(5, ambient_strength);

        m_chunk_mesh_pool.for_each_chunk_mesh(
            [&](const ChunkMeshData& chunk_mesh)
            {
                if (chunk_mesh.vertex_count > 0)
                {
                    pipeline->set_uniform_ivec3(1, chunk_mesh.chunk_pos);
                    renderer.draw_arrays(chunk_mesh.vertex_array, chunk_mesh.vertex_count, gfx::DrawMode::Triangles);
                }
            }
        );
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

    void ChunkClient::trim_far_chunks()
    {
        // m_chunk_mgr.erase_far_chunks({ m_previous_player_chunk_col_pos }, m_voxel_bounds.bounds_distance());
    }

    void ChunkClient::rebuild_chunk_mesh(const v3i& chunk_pos)
    {
        // Calculate the job's priority
        const auto player = m_scene->get_actor_by_tag(ActorTag::LocalPlayer);
        f32 job_priority = 0.0f;
        if (player)
        {
            const v3 chunk_world_pos = voxel_utils::chunk_to_world_pos(chunk_pos);
            const v3 player_pos = player->transform.position;
            job_priority = glm::distance2(chunk_world_pos, player_pos);
        }

        g_engine->thread_pool().queue_job(job_priority,
            [this, chunk_pos]
            {
                // Get all neighbouring chunks
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

                m_chunk_mgr.view<v3u{3}>(chunk_pos - v3i{ 1 },
                    [&](const auto& chunk_view)
                    {
                        m_chunk_mesh_pool.build_chunk_mesh(chunk_view);
                    }
                );
            }
        );
    }
}
