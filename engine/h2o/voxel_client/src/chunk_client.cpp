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

                auto chunk_column = std::make_shared<ChunkColumn>(chunk_pos);
                std::vector<size_t> mesh_indices{}; // The indices of chunk meshes into the mesh pool
                for (size_t i = 0; i < voxel_constants::vertical_chunk_count; i++)
                {
                    auto& chunk = (*chunk_column)[i];
                    chunk.init(*m_voxel_module);
                    chunk.decompress(compressed_chunks[i]);

                    if (!chunk.is_empty())
                    {
                        // Reserve a chunk mesh, as this chunk is in range.
                        auto [chunk_mesh, mesh_index] = find_available_chunk_mesh();
                        chunk_mesh.is_available = false;

                        mesh_indices.push_back(mesh_index);
                        chunk_mesh.chunk_mesh.init(
                            *m_voxel_rendering_module,
                            m_rendering_module->renderer());

                        chunk_mesh.chunk_mesh.update(chunk, {});
                    }
                }

                m_chunk_columns[chunk_pos] = { chunk_column, mesh_indices };
            }
        );
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
            if (chunk_mesh.is_ready())
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

        for (i32 i = -m_view_distance; i <= m_view_distance; i++)
        for (i32 j = -m_view_distance; j <= m_view_distance; j++)
        {
            const v2i chunk_pos {
                m_previous_player_chunk_col_pos.x + i,
                m_previous_player_chunk_col_pos.y + j };

            const auto it = m_chunk_columns.find(chunk_pos);
            if (it != m_chunk_columns.end())
                continue;

            m_chunk_columns[chunk_pos] = ChunkData { nullptr, {} };
            chunk_fetch_request.requested_chunks.push_back(chunk_pos);
        }

        m_client->send_message(0, chunk_fetch_request);
    }

    void ChunkClient::trim_far_chunks()
    {
        erase_if(m_chunk_columns,
            [&](const auto& item) -> bool
            {
                return !is_in_range(item.first);
            }
        );
    }

    std::pair<ChunkClient::ChunkMeshData&, size_t> ChunkClient::find_available_chunk_mesh()
    {
        size_t index = 0;
        for (; index < m_chunk_mesh_pool.size(); index++)
        {
            auto& val = m_chunk_mesh_pool[index];
            if (val.is_available)
                return { val, index };
        }

        auto& val = m_chunk_mesh_pool.emplace_back();
        return { val, index };
    }
}