#include "scene/scene.h"

#include "core/engine.h"
#include "networking/net_peer.h"
#include "scene/scene_module.h"
#include "scene/scene_net_messages.h"

namespace h2o
{
    Scene::Scene(const std::string& scene_name, INetPeer* net_peer)
        : Tickable(g_engine)
        , m_scene_name(scene_name)
        , m_net_peer(net_peer)
    {
        if (net_peer)
        {
            net_peer->handle_message<net_msg::ActorDestroyed>(m_on_object_destroyed_handle,
                [&](PeerID peer_id, const net_msg::ActorDestroyed& actor_destroyed_msg)
                {
                    destroy_actor(actor_destroyed_msg.actor_id, false);
                }
            );
        }

        if (auto scene_module = g_engine->get_module<SceneModule>())
            scene_module->register_scene(*this);
    }

    Scene::~Scene()
    {
        if (auto scene_module = g_engine->get_module<SceneModule>())
            scene_module->unregister_scene(*this);

        m_actor_map.clear();
        m_system_map.clear();
    }

    bool Scene::init()
    {
        if (auto scene_module = g_engine->get_module<SceneModule>())
            scene_module->register_scene(*this);

        bool success = true;
        for (const auto& system : m_system_map)
        {
            if (!system.second || !system.second->init())
                success = false;
        }

        if (!success)
            log::warn("Failed to initialize all systems for scene: {}", m_scene_name);

        return success;
    }

    void Scene::cleanup()
    {
        m_actor_map.clear();
        m_actor_tags.clear();
        m_system_map.clear();
    }

    bool Scene::destroy_actor(ActorID actor_id, bool replicate)
    {
        if (m_actor_map.erase(actor_id) == 0)
            return false;

        if (replicate && m_net_peer)
        {
            for (h2o::PeerID peer_id : m_net_peer->peers())
                m_net_peer->send_message(peer_id, net_msg::ActorDestroyed{ actor_id });
        }

        return true;
    }

    void Scene::tag_actor(const WeakHandle<Actor>& actor, ActorTag tag)
    {
        const ActorTag previous_tag = actor->get_tag();
        if (tag == previous_tag)
            return;

        m_actor_tags[previous_tag] = nullptr;
        if (tag != ActorTag::None)
            m_actor_tags[tag] = actor;

        actor->m_actor_tag = tag;
    }
}