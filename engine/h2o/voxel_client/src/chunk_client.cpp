#include "voxel_client/chunk_client.h"

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
        set_tick_phases(Update);

        m_client->on_connected_to_server.add_listener(m_on_connected_handle,
            [&](const Client::OnConnectedEvent& on_connected_event)
            {
                m_refresh_chunk_requests = true;
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

            m_chunks[chunk_pos] = nullptr;
            chunk_fetch_request.requested_chunks.push_back(chunk_pos);
        }

        m_client->send_message(0, chunk_fetch_request);
    }

    void ChunkClient::trim_far_chunks()
    {
        const i32 valid_dist = m_view_distance + m_stay_loaded_distance;
        const v2i min = m_previous_player_chunk_col_pos - v2i{ valid_dist, valid_dist };
        const v2i max = m_previous_player_chunk_col_pos + v2i{ valid_dist, valid_dist };

        erase_if(m_chunks,
            [min, max](const auto& item) -> bool
            {
                const v2i chunk_col_pos = item.first;
                return
                    chunk_col_pos.x < min.x || chunk_col_pos.x > max.x ||
                    chunk_col_pos.y < min.y || chunk_col_pos.y > max.y;
            }
        );
    }
}