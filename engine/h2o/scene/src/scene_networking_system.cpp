#include "scene/scene_networking_system.h"

#include "networking/client.h"
#include "scene/actor.h"
#include "scene/scene.h"
#include "scene/scene_net_messages.h"

namespace h2o
{
    SceneNetworkingSystem::SceneNetworkingSystem(const SceneSystemInitializer& system_initializer, Client& client)
        : SceneSystem(system_initializer)
        , m_client(&client)
    {
        set_tick_phases(TickPhase::Update);

        m_client->handle_message<h2o::net_msg::TransformUpdate>(m_on_received_transform_update_handle,
            [&](h2o::ClientID client_id, const h2o::net_msg::TransformUpdate& transform_update)
            {
                if (auto actor = m_scene->get_actor(transform_update.actor_id))
                    actor->transform = transform_update.transform;
            }
        );
    }

    void SceneNetworkingSystem::update(f32 delta_time)
    {
        if (!m_client || !m_client->is_connected())
            return;

        for (const auto& actor : m_replicated_actors)
        {
            if (actor)
            {
                Transform transform = actor->transform;
                transform.rotation.x = 0.0f;
                transform.rotation.z = 0.0f;
                transform.rotation.y -= 90.0f;
                transform.rotation.y *= -1.0f;

                m_client->send_message(0, net_msg::TransformUpdate { 0, transform });
            }
        }
    }

    void SceneNetworkingSystem::replicate_actor_transform(const WeakHandle<Actor>& actor)
    {
        m_replicated_actors.push_back(actor);
    }

    void SceneNetworkingSystem::stop_replicating_actor_transform(const WeakHandle<Actor>& actor)
    {
        std::erase(m_replicated_actors, actor);
    }
}