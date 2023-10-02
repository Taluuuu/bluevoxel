#include "voxel_client/chunk_client.h"

#include "core/engine.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "voxel/voxel_net_messages.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    ChunkClient::ChunkClient(
        const SceneSystemInitializer& system_initializer,
        Client& client)
        : SceneSystem(system_initializer)
        , m_client(&client)
    {
        set_tick_phases(TickPhase_Update | TickPhase_Render);

        auto voxel_module = g_engine->get_module<VoxelModule>();
        assert(voxel_module);

        auto rendering_module = g_engine->get_module<RenderingModule>();
        assert(rendering_module);

        m_client->on_connected_to_server.add_listener(m_on_connected_handle,
            [&](const Client::OnConnectedEvent& on_connected_event)
            {
                m_refresh_chunk_requests = true;
            }
        );

        m_client->handle_message<NetMsg_ChunkFetchResult>(m_on_fetched_chunk_handle,
            [&, voxel_module](ClientID client_id, const NetMsg_ChunkFetchResult& chunk_fetch_result)
            {
                auto& [compressed_chunks, chunk_pos] = chunk_fetch_result;

                if (!is_in_range(chunk_pos))
                    return;

                if (compressed_chunks.size() != voxel_constants::vertical_chunk_count)
                    return;

                auto chunk_column = std::make_shared<ChunkColumn>(chunk_pos);
                for (size_t i = 0; i < voxel_constants::vertical_chunk_count; i++)
                {
                    auto& chunk = (*chunk_column)[i];
                    chunk.init(*voxel_module);
                    chunk.decompress(compressed_chunks[i]);
                }

                // Reserve a chunk mesh, as this chunk is in range.
                auto [chunk_mesh, mesh_index] = find_available_chunk_mesh();
                m_chunks[chunk_pos] = { chunk_column, mesh_index };
                chunk_mesh.is_available = false;

                chunk_mesh.chunk_mesh.init(*voxel_rendering_module, renderer, );
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

        for (const auto& [chunk_mesh_column, _] : m_chunk_mesh_pool)
        {
            for (const auto& chunk_mesh : chunk_mesh_column)
            {
                if (chunk_mesh.is_ready())
                {

    //                chunk_mesh
                }
            }
        }
    }

    bool ChunkClient::is_in_range(v2i chunk_pos) const
    {
        const i32 valid_dist = m_view_distance + m_stay_loaded_distance;
        const v2i min = m_previous_player_chunk_col_pos - v2i { valid_dist, valid_dist };
        const v2i max = m_previous_player_chunk_col_pos + v2i { valid_dist, valid_dist };

        return
            chunk_pos.x < min.x || chunk_pos.x > max.x ||
            chunk_pos.y < min.y || chunk_pos.y > max.y;
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

            const auto it = m_chunks.find(chunk_pos);
            if (it != m_chunks.end())
                continue;

            m_chunks[chunk_pos] = ChunkData { nullptr, 0 };
            chunk_fetch_request.requested_chunks.push_back(chunk_pos);
        }

        m_client->send_message(0, chunk_fetch_request);
    }

    void ChunkClient::trim_far_chunks()
    {
        erase_if(m_chunks,
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