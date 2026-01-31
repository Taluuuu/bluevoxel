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
                    const auto& buffer = update_component_msg.serialized_components;
                    net_utils::Reader reader{ buffer.begin(), buffer.size() };

                    for (const u32 entity_id : update_component_msg.entity_ids)
                    {
                        const auto it = m_server_to_local.find(entity_id);
                        if (it == m_server_to_local.end())
                        {
                            log::warn("Could not update component on entity {}: local entity not found.", entity_id);
                            m_scene_module.skip_component(update_component_msg.component_type, reader);
                            continue;
                        }

                        // TODO: Find a better way for this. Just ignore component updates for locally controlled entities
                        if (const auto sync = m_registry.try_get<NetworkSync>(it->second))
                        {
                            if (sync->owner == local_peer_id())
                            {
                                m_scene_module.skip_component(update_component_msg.component_type, reader);
                                continue;
                            }
                        }

                        m_scene_module.deserialize_component(m_registry, it->second, update_component_msg.component_type, reader, m_net_peer->is_host());
                    }
                }
            );
        }

        set_tick_phases(TickPhase::NetworkUpdate | TickPhase::PostUpdate);

        m_scene_module.register_scene(*this);
    }

    Scene::~Scene()
    {
        m_scene_module.unregister_scene(*this);
        m_system_map.clear();
    }

    u32 Scene::local_peer_id() const
    {
        if (const auto peer = net_peer())
            return peer->local_peer_id();

        return 0;
    }

    SceneSystemInitializer Scene::make_system_initializer()
    {
        return SceneSystemInitializer
        {
            .owning_scene = *this,
            .is_host = m_net_peer ? m_net_peer->is_host() : true
        };
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
                        {
                            m_net_peer->send_message(peer_id, msg);
                            // log::info("SEND UPDATE to {}", peer_id);
                        }
                    }
                }
            );
        }
    }

    void Scene::on_network_sync_created(const entt::entity entity)
    {
        // If server, assign an entity id
        if (net_peer()->is_host())
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
