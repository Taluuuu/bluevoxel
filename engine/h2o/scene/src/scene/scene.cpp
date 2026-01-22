#include "scene/scene.h"

#include "core/engine.h"
#include "networking/net_peer.h"
#include "scene/scene_module.h"
#include "scene/scene_networking_components.h"
#include "scene/scene_net_messages.h"

#include <utility>

#include "networking/client.h"
#include "networking/server.h"

namespace h2o
{
    Scene::Scene(std::string scene_name, INetPeer* net_peer)
        : Tickable(g_engine)
        , m_scene_name(std::move(scene_name))
        , m_net_peer(net_peer)
        , m_scene_module(g_engine->get_module_checked<SceneModule>())
    {
        m_registry.on_construct<NetworkSync>().connect<&Scene::on_network_sync_created>(*this);

        if (auto* server = dynamic_cast<Server*>(net_peer))
        {
            server->on_player_joined.add_listener(m_on_player_joined_handle,
                [this](const Server::PlayerConnectionChangedEvent& event)
                {
                    m_registry.view<NetworkSync>().each(
                        [&](const entt::entity entity, const NetworkSync&)
                        {
                            net_utils::Buffer buffer{};
                            net_utils::Writer writer{ buffer };

                            m_scene_module.serialize_entity(m_registry, entity, writer);

                            const net_msg::SpawnEntity msg{ buffer };
                            m_net_peer->send_message(event.client_id, msg);
                        }
                    );
                }
            );
        }
        else if (auto* client = dynamic_cast<Client*>(net_peer))
        {
            client->handle_message<net_msg::SpawnEntity>(m_on_spawn_entity_handle,
            [this](PeerID, const net_msg::SpawnEntity& spawn_entity_msg)
                {
                    const auto& buffer = spawn_entity_msg.serialized_components;
                    net_utils::Reader reader{ buffer.begin(), buffer.size() };

                    const auto entity = m_scene_module.deserialize_entity(m_registry, reader);
                    if (const auto sync = m_registry.try_get<NetworkSync>(entity))
                    {
                        m_server_to_local[sync->entity_id] = entity;
                        log::info("Spawned entity with id: {}", sync->entity_id);
                    }
                }
            );
        }

        if (net_peer)
        {
            net_peer->handle_message<net_msg::UpdateComponent>(m_on_update_component_handle,
                [this](PeerID, const net_msg::UpdateComponent& update_component_msg)
                {
                    for (const u32 entity_id : update_component_msg.entity_ids)
                    {
                        // TODO: Find a better way for this. Just ignore component updates for locally controlled entities
                        if (entity_id == local_peer_id())
                            continue;

                        const auto it = m_server_to_local.find(entity_id);
                        if (it == m_server_to_local.end())
                        {
                            log::warn("Could not update component on entity {}: local entity not found.", entity_id);
                            continue;
                        }

                        const auto& buffer = update_component_msg.serialized_components;
                        net_utils::Reader reader{ buffer.begin(), buffer.size() };

                        m_scene_module.deserialize_component(m_registry, it->second, update_component_msg.component_type, reader);
                    }
                }
            );

            m_local_peer_id = net_peer->local_peer_id();
        }

        set_tick_phases(TickPhase::FrameStart | TickPhase::NetworkUpdate);

        m_scene_module.register_scene(*this);
    }

    Scene::~Scene()
    {
        m_scene_module.unregister_scene(*this);
        m_system_map.clear();
    }

    SceneSystemInitializer Scene::make_system_initializer()
    {
        return SceneSystemInitializer
        {
            .owning_scene = *this,
            .is_host = m_net_peer ? m_net_peer->is_host() : true
        };
    }

    void Scene::frame_start()
    {
        for (const auto& actor : m_actors_to_run_start)
            actor->start();
        m_actors_to_run_start.clear();
    }

    void Scene::network_update(f32 delta_time)
    {
        if (!m_net_peer)
            return;

        for (const entt::entity entity : m_newly_spawned_entities)
            on_network_sync_entity_created.broadcast(entity);

        // This runs only on the server, as it is (for now?) the only authority on spawning entities
        if (!m_newly_spawned_entities.empty() && m_net_peer->is_host())
        {
            // Send newly spawned entities since last network update to peers
            net_utils::Buffer buffer{};
            net_utils::Writer writer{ buffer };
            for (const entt::entity entity : m_newly_spawned_entities)
                m_scene_module.serialize_entity(m_registry, entity, writer);

            const net_msg::SpawnEntity msg{ buffer };
            for (const PeerID peer_id : m_net_peer->peers())
                m_net_peer->send_message(peer_id, msg);
        }
        m_newly_spawned_entities.clear();

        {
            // Serialize all dirty components, and send updates to peers
            m_scene_module.for_each_component_type(
                [&](const entt::id_type component_type)
                {
                    net_utils::Buffer buffer{};
                    net_utils::Writer writer{ buffer };

                    std::vector<u32> updated_entity_ids{};
                    m_scene_module.serialize_dirty_components(m_registry, component_type, updated_entity_ids, writer);

                    if (!updated_entity_ids.empty())
                    {
                        const net_msg::UpdateComponent msg{ component_type, updated_entity_ids, buffer };
                        for (const PeerID peer_id : m_net_peer->peers())
                            m_net_peer->send_message(peer_id, msg);
                    }
                }
            );
        }
    }

    void Scene::on_network_sync_created(const entt::entity entity)
    {
        // If server, assign an entity id
        if (m_local_peer_id == 0)
        {
            if (const auto sync = m_registry.try_get<NetworkSync>(entity))
            {
                sync->entity_id = m_entity_id_generator++;
                m_server_to_local[sync->entity_id] = entity;
            }
        }

        m_newly_spawned_entities.emplace_back(entity);
    }
}
