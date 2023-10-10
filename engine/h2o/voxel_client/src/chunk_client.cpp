#include "voxel_client/chunk_client.h"

#include "core/engine.h"
#include "rendering/camera.h"
#include "rendering/pipeline.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture_array.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
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
    {
        set_tick_phases(TickPhase_Update | TickPhase_Render);

        m_rendering_module       = &g_engine->get_module_checked<RenderingModule>();
        m_voxel_module           = &g_engine->get_module_checked<VoxelModule>();
        m_voxel_rendering_module = &g_engine->get_module_checked<VoxelRenderingModule>();

        m_client->on_connected_to_server.add_listener(m_on_connected_handle,
            [&](const Client::OnConnectedEvent& on_connected_event)
            {
                m_refresh_chunk_requests = true;
            }
        );

        m_client->handle_message<NetMsg_ChunkFetchResult>(m_on_fetched_chunk_handle,
            [&](ClientID client_id, const NetMsg_ChunkFetchResult& chunk_fetch_result)
            {
                auto& [compressed_chunks, chunk_pos] = chunk_fetch_result;

                if (!is_in_range(chunk_pos))
                    return;

                if (compressed_chunks.size() != voxel_constants::vertical_chunk_count)
                    return;

                // Decompressing a chunk is slow. Run it on a thread.
                g_engine->thread_pool().queue_job(
                    [&, compressed_chunks, chunk_pos]()
                    {
                        auto chunk_column = std::make_shared<ChunkColumn>(chunk_pos);

                        // Decompress chunks
                        std::vector<v3i> chunks_to_mesh{};
                        for (size_t i = 0; i < voxel_constants::vertical_chunk_count; i++)
                        {
                            auto& chunk = (*chunk_column)[i];
                            chunk.init(*m_voxel_module);
                            chunk.decompress(compressed_chunks[i]);

                            if (!chunk.is_empty())
                                chunks_to_mesh.push_back(chunk.chunk_pos());
                        }

                        // Add chunk column to the world
                        {
                            std::lock_guard chunk_columns_lock(m_chunk_columns_mutex);
                            auto& chunk_data = m_chunk_columns[chunk_pos];
                            chunk_data.chunk_column = std::move(chunk_column);
                        }

                        // Enqueue chunks to build meshes
                        {
                            std::lock_guard chunks_to_mesh_lock(m_chunks_to_mesh_mutex);
                            m_chunks_to_mesh.insert(
                                m_chunks_to_mesh.end(),
                                chunks_to_mesh.begin(), chunks_to_mesh.end());
                        }
                    }
                );
            }
        );

        m_client->handle_message<NetMsg_BlockPlaceRequest>(m_on_received_block_place_request,
            [&](ClientID client_id, const NetMsg_BlockPlaceRequest& block_place_request)
            {
                if (set_block_at(block_place_request.block_pos, block_place_request.placed_block))
                    m_chunks_to_mesh.push_back(voxel_utils::block_to_chunk_pos(block_place_request.block_pos));
            }
        );
    }

    void ChunkClient::set_block_at_replicated(const v3i& block_pos, Block block)
    {
        assert(m_client);
        if (set_block_at(block_pos, block))
        {
            m_client->send_message(0, NetMsg_BlockPlaceRequest { block, block_pos });
            m_chunks_to_mesh.push_back(voxel_utils::block_to_chunk_pos(block_pos));
        }
    }

    void ChunkClient::set_block_at_replicated(Chunk& chunk, const v3i& block_pos, Block block)
    {
        chunk.set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
        m_client->send_message(0, NetMsg_BlockPlaceRequest { block, block_pos });
        m_chunks_to_mesh.push_back(chunk.chunk_pos());
    }

    Chunk* ChunkClient::get_chunk_at(const v3i& chunk_pos)
    {
        auto it = m_chunk_columns.find({ chunk_pos.x, chunk_pos.z });
        if (it == m_chunk_columns.end())
            return nullptr;

        if (const auto& chunk_col = it->second.chunk_column)
            return chunk_col->get_chunk_safe(chunk_pos.y);

        return nullptr;
    }

    const Chunk* ChunkClient::get_chunk_at(const v3i& chunk_pos) const
    {
        auto it = m_chunk_columns.find({ chunk_pos.x, chunk_pos.z });
        if (it == m_chunk_columns.end())
            return nullptr;

        if (const auto& chunk_col = it->second.chunk_column)
            return chunk_col->get_chunk_safe(chunk_pos.y);

        return nullptr;
    }

    void ChunkClient::update(f32 delta_time)
    {
        if (!m_client->is_connected())
            return;

        // Bad
        auto player = m_scene->get_actor("player");
        if (!player)
            return;

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

        build_chunk_meshes(1, player_pos);
    }

    void ChunkClient::render(f32 delta_time)
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

        // TODO: Baddd
        const m4 proj_view = camera->calc_proj_view();
        pipeline->set_uniform_mat4(0, proj_view);

        block_textures->bind(0);
        pipeline->set_uniform_int(2, 0);

        for (const auto& [chunk_mesh, _] : m_chunk_mesh_pool)
        {
            if (!chunk_mesh.is_empty())
            {
                pipeline->set_uniform_ivec3(1, chunk_mesh.chunk_pos());
                renderer.draw(chunk_mesh.vertex_array(), chunk_mesh.vertex_count());
            }
        }
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
        NetMsg_ChunkFetchRequest chunk_fetch_request{};

        {
            std::lock_guard lock(m_chunk_columns_mutex);
            for (i32 i = -m_view_distance; i <= m_view_distance; i++)
            for (i32 j = -m_view_distance; j <= m_view_distance; j++)
            {
                const v2i chunk_pos {
                    m_previous_player_chunk_col_pos.x + i,
                    m_previous_player_chunk_col_pos.y + j };

                const auto it = m_chunk_columns.find(chunk_pos);
                if (it != m_chunk_columns.end())
                    continue;

                m_chunk_columns[chunk_pos] = ChunkData{};
                chunk_fetch_request.requested_chunks.push_back(chunk_pos);
            }
        }

        m_client->send_message(0, chunk_fetch_request);
    }

    void ChunkClient::trim_far_chunks()
    {
        std::lock_guard lock(m_chunk_columns_mutex);

        erase_if(m_chunk_columns,
            [&](const auto& item) -> bool
            {
                return !is_in_range(item.first);
            }
        );
    }

    void ChunkClient::build_chunk_meshes(i32 max_chunk_meshes, const v3& player_pos)
    {
        auto pop_nearest_chunk_to_mesh =
            [&]() -> std::optional<v3i>
            {
                f32 nearest_distance_sqr = FLT_MAX;
                i32 nearest_idx = -1;
                std::optional<v3i> nearest_chunk_pos = std::nullopt;

                for (i32 i = 0; i < m_chunks_to_mesh.size(); i++)
                {
                    const v3i& chunk_to_remesh = m_chunks_to_mesh[i];
                    const f32 distance_with_chunk_sqr = glm::distance2(v3(chunk_to_remesh), player_pos);

                    if (distance_with_chunk_sqr < nearest_distance_sqr)
                    {
                        nearest_distance_sqr = distance_with_chunk_sqr;
                        nearest_chunk_pos = chunk_to_remesh;
                        nearest_idx = i;
                    }
                }

                if (nearest_chunk_pos)
                    m_chunks_to_mesh.erase(m_chunks_to_mesh.cbegin() + nearest_idx);

                return nearest_chunk_pos;
            };

        std::lock_guard lock(m_chunks_to_mesh_mutex);
        for (i32 i = 0; i < max_chunk_meshes; i++)
        {
            if (const auto chunk_to_mesh = pop_nearest_chunk_to_mesh())
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
        std::lock_guard lock(m_chunk_columns_mutex);

        const v2i chunk_col_pos { chunk_pos.x, chunk_pos.z };
        const auto it = m_chunk_columns.find(chunk_col_pos);
        if (it == m_chunk_columns.end())
            return;

        auto& [chunk_column, chunk_mesh_indices] = it->second;
        assert(chunk_column);

        const auto& chunk = (*chunk_column)[chunk_pos.y];
        i32& chunk_mesh_index = chunk_mesh_indices[chunk_pos.y];

        if (chunk_mesh_index == -1)
        {
            if (chunk.is_empty())
            {
                // No need for a chunk mesh.
                return;
            }
            else
            {
                auto [mesh_data, mesh_index] = reserve_chunk_mesh();
                chunk_mesh_index = mesh_index;

                assert(m_voxel_rendering_module && m_rendering_module);
                mesh_data.chunk_mesh.init(
                    *m_voxel_rendering_module,
                    m_rendering_module->renderer());

                assert(mesh_index != -1);
            }
        }

        assert(chunk_column);
        assert(chunk_mesh_index < m_chunk_mesh_pool.size());

        m_chunk_mesh_pool[chunk_mesh_index].chunk_mesh.update(chunk, {});
    }

    std::pair<ChunkClient::ChunkMeshData&, i32> ChunkClient::reserve_chunk_mesh()
    {
        i32 index = 0;
        for (; index < m_chunk_mesh_pool.size(); index++)
        {
            auto& chunk_mesh_data = m_chunk_mesh_pool[index];
            if (chunk_mesh_data.is_available)
            {
                chunk_mesh_data.is_available = false;
                return { chunk_mesh_data, index };
            }
        }

        auto& chunk_mesh_data = m_chunk_mesh_pool.emplace_back();
        chunk_mesh_data.is_available = false;

        return { chunk_mesh_data, index };
    }
}