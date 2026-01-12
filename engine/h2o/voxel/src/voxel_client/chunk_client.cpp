#include "voxel_client/chunk_client.h"

#include "core/engine.h"
#include "core/profiling/scope_timer.h"
#include "networking/net_peer.h"
#include "physics/scene/physics_system.h"
#include "rendering/camera.h"
#include "rendering/pipeline.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture_array.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "ui/imgui.h"
#include "voxel/chunk_column_heightmap.h"
#include "voxel/chunk_region.h"
#include "voxel/compressed_chunk.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_net_messages.h"
#include "voxel/voxel_ray.h"
#include "voxel/voxel_utils.h"
#include "weather/weather_system.h"

#include <glm/gtx/norm.hpp>
#include <magic_enum/magic_enum_all.hpp>

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
            [&](PeerID, const net_msg::ChunkFetchResult& chunk_fetch_result)
            {
                // ScopeTimer timer("Received chunk column");
                auto& [compressed_chunks, chunk_column_pos] = chunk_fetch_result;
                log::info("Received chunk column at {}", chunk_column_pos);

                if (!m_voxel_bounds.in_bounds(chunk_column_pos))
                    return;

                if (compressed_chunks.size() != voxel_constants::vertical_chunk_count)
                    return;

                ++m_num_chunk_columns_pending_decompress;

                // Decompressing a chunk is slow. Run it on a thread.
                v2 player_pos_2d{ m_player_pos.x, m_player_pos.z };
                // log::info("QUEUE JOB 'DECOMPRESS CHUNKS'");
                g_engine->thread_pool().queue_job(glm::distance(player_pos_2d, voxel_utils::chunk_to_world_pos(chunk_column_pos)),
                    [this, compressed_chunks]()
                    {
                        // TODO: A vector of compressed chunks is always a chunk column, so the class
                        //       should be CompressedChunkColumn instead so I don't have to fetch the
                        //       column at every iteration

                        std::unordered_set<v3i> updated_chunks{};
                        for (const auto& compressed_chunk : compressed_chunks)
                        {
                            const v3i chunk_pos = compressed_chunk.chunk_pos();
                            m_chunk_mgr.fetch_mut<Chunk>(chunk_pos,
                                [&](Chunk* chunk)
                                {
                                    assert(chunk != nullptr);
                                    compressed_chunk.decompress(*chunk);
                                    updated_chunks.insert(chunk_pos);
                                }, true, false
                            );
                        }

                        m_chunk_mgr.cells_updated_event<Chunk>().broadcast({ updated_chunks });

                        --m_num_chunk_columns_pending_decompress;
                    }
                );
            }
        );

        m_client->handle_message<net_msg::BlockPlaceRequest>(m_on_received_block_place_request_handle,
            [this](PeerID client_id, const net_msg::BlockPlaceRequest& block_place_request)
            {
                m_chunk_mgr.set_block_at(block_place_request.block_pos, block_place_request.placed_block);
            }
        );

        // TODO: Move these in a common server/client base class, as they will certainly be needed server-side as well
        if (const auto physics_system = scene.get_system<PhysicsSystem>())
        {
            physics_system->on_testing_collisions.add_listener(m_on_testing_collisions_handle,
                [this](const TestingCollisionEvent& event)
                {
                    const auto [col_pos, col_size] = event.collider_to_test;

                    const v3i min = voxel_utils::world_to_block_pos(col_pos);
                    const v3i max = voxel_utils::world_to_block_pos(col_pos + col_size) + v3i{1};

                    voxel_utils::for_v3i(min, max,
                        [&](const v3i& block_pos)
                        {
                            if (const auto block = m_chunk_mgr.get_block_at(block_pos); block != Block::Air)
                                event.near_colliders.emplace_back(block_pos, v3{1.0f});
                        }
                    );
                }
            );

            physics_system->on_raycasting.add_listener(m_on_raycasting_handle,
                [this](const RaycastingEvent& event)
                {
                    const VoxelRay voxel_ray(event.origin, event.origin + event.direction * event.range, m_chunk_mgr);
                    if (voxel_ray.has_hit())
                    {
                        const auto& voxel_ray_hit = voxel_ray.hit();
                        if (voxel_ray_hit.distance < event.hit_result.distance)
                        {
                            event.hit_result.distance = voxel_ray_hit.distance;
                            event.hit_result.hit_location = event.origin + event.direction * voxel_ray_hit.distance;
                        }
                    }
                }
            );
        }

        // TODO: This should not be necessary to bridge between the voxel world renderer and the weather system.
        if (const auto weather_system = scene.get_system<WeatherSystem>())
        {
            weather_system->on_time_changed.add_listener(m_on_time_changed_handle,
                [this](const i64)
                {
                    if (const auto weather_system = scene.get_system<WeatherSystem>())
                    {
                        m_voxel_world_renderer.sun_brightness = weather_system->current_lighting_settings().brightness;
                        m_voxel_world_renderer.light_color = weather_system->current_lighting_settings().light_color;

                        // Make sure light_dir.y is always negative for a better looking night-time
                        v3 light_dir = -weather_system->sun_direction();
                        light_dir.y = glm::min(-0.5f, light_dir.y);
                        light_dir = glm::normalize(light_dir);
                        m_voxel_world_renderer.light_dir = light_dir;
                    }
                }
            );
        }
    }

    void ChunkClient::reload_all()
    {
        m_chunk_mgr.remove_all([](v2i){ return true; });
        request_chunk_loads();
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

        auto player = scene.get_actor_by_tag(ActorTag::LocalPlayer);
        if (!player)
            return;

        m_chunk_mgr.set_player_positions({ player->transform.position });

        m_player_pos = player->transform.position;
        const v3i player_chunk_pos = voxel_utils::world_to_chunk_pos(m_player_pos);
        const v2i player_chunk_col_pos { player_chunk_pos.x, player_chunk_pos.z };
        m_voxel_bounds.set_bounds_center(player_chunk_col_pos);

        m_voxel_world_renderer.player_pos = m_player_pos;

        if (player_chunk_col_pos != m_previous_player_chunk_col_pos || m_refresh_chunk_requests)
        {
            m_previous_player_chunk_col_pos = player_chunk_col_pos;

            m_chunk_mgr.remove_all(
                [&](const v2i chunk_column_pos) -> bool
                {
                    return !m_voxel_bounds.in_bounds(chunk_column_pos);
                }
            );

            request_chunk_loads();
        }

        m_refresh_chunk_requests = false;

        g_engine->debug_infos().update_debug_statistic(
            "voxels", "chunk columns pending decompress", m_num_chunk_columns_pending_decompress);

        m_chunk_mgr.broadcast_events();

        // Heightmap debug if ever needed
        // m_chunk_mgr.fetch<Chunk>(v3i{0},
        //     [&](const Chunk* chunk)
        //     {
        //         if (!chunk)
        //             return;
        //
        //         const auto& heightmap = chunk->column_heightmap();
        //         if (!heightmap)
        //             return;
        //
        //         for (i32 i = 0; i < 32; i++)
        //         for (i32 j = 0; j < 32; j++)
        //         {
        //             const u32 height = heightmap->get_height({ i, j });
        //             m_rendering_module->renderer().draw_sphere(v3{ i, height, j } + v3{ 0.5f, 0.0f, 0.5f }, 0.2f, v4{1.0f});
        //         }
        //     }
        // );

        if (ImGui::Begin("Chunk Debug"))
        {
            const auto draw_list = ImGui::GetWindowDrawList();
            m_voxel_bounds.for_each_pos_in_bounds(
                [&](const v2i chunk_col_pos)
                {
                    const bool is_generated = m_chunk_mgr.is_chunk_column_generated(chunk_col_pos);
                    const bool can_be_meshed = m_chunk_mgr.is_ready_for_meshing({ chunk_col_pos.x, 0, chunk_col_pos.y });
                    u32 square_color = is_generated ? ImColor(0.0f, 1.0f, 0.0f, 1.0f) : ImColor(1.0f, 0.0f, 0.0f, 1.0f);
                    if (is_generated && !can_be_meshed)
                        square_color = ImColor(1.0f, 1.0f, 0.0f, 1.0f);

                    const v2i local_chunk_col_pos = chunk_col_pos - m_voxel_bounds.min();
                    constexpr f32 square_size = 10.0f;
                    constexpr f32 square_spacing = 2.0f;
                    draw_list->AddRectFilled(
                        v2{ ImGui::GetCursorScreenPos() } + (square_size + square_spacing) * v2{ local_chunk_col_pos },
                        v2{ ImGui::GetCursorScreenPos() } + (square_size + square_spacing) * v2{ local_chunk_col_pos } + v2{ square_size },
                        square_color
                    );
                }
            );
        }
        ImGui::End();
    }

    void ChunkClient::request_chunk_loads()
    {
        net_msg::ChunkFetchRequest chunk_fetch_request{};

        m_voxel_bounds.for_each_pos_in_bounds(
            [&](const v2i chunk_column_pos)
            {
                if (!m_chunk_mgr.cell_column_exists(chunk_column_pos))
                    chunk_fetch_request.requested_chunks.push_back(chunk_column_pos);
            }
        );

        if (!chunk_fetch_request.requested_chunks.empty())
            m_client->send_message(0, chunk_fetch_request);
    }
}
