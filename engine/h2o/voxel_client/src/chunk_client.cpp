#include "voxel_client/chunk_client.h"

#include "scene/scene.h"
#include "voxel/voxel_net_messages.h"

namespace h2o
{
    ChunkClient::ChunkClient(
        const SceneSystemInitializer& system_initializer,
        Client& client)
        : SceneSystem(system_initializer)
        , m_client(&client)
    {
        set_tick_phases(Update);
    }

    void ChunkClient::update(f32 delta_time)
    {
        // Bad
        auto player = m_scene->get_actor("player");
        if (!player) return;

        const v3& player_pos = player->transform.position;

        if (m_client->is_connected())
        {
            NetMsg_ChunkFetchRequest chunk_fetch_request{ { { 69, -69 } } };
            m_client->send_message(chunk_fetch_request);
            temp = false;
        }
    }
}