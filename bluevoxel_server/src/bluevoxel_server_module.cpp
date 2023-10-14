#include "bluevoxel_server_module.h"

#include "core/engine.h"
#include "networking/networking_module.h"
#include "networking/networking_utils.h"
#include "scene/scene.h"
#include "scene/scene_net_messages.h"
#include "voxel/chunk_generators/chunk_generator_flat.h"
#include "voxel_server/chunk_server.h"
#include "voxel_server/voxel_server_module.h"

#include <vector>

namespace bluevoxel
{
    bool BlueVoxelServerModule::init(h2o::Engine& engine)
    {
        if (!m_server.start(1338))
            return false;

        // Setup scene
        m_scene = std::make_shared<h2o::Scene>("server_scene");

        // Setup chunk server
        m_chunk_server = std::make_unique<h2o::ChunkServer>(m_server);
        auto chunk_generator = std::make_unique<h2o::ChunkGenerator_Flat>();
        chunk_generator->block_layers = { 3, 3, 3, 3, 3, 2, 2, 2, 1 };
        m_chunk_server->set_chunk_generator(std::move(chunk_generator));
        m_chunk_server->start();

        m_server.on_player_joined.add_listener(m_player_joined_event_handle,
            [&](const h2o::Server::OnPlayerJoinedEvent& event)
            {
                // Use client id as actor id
                m_scene->spawn_actor(h2o::Transform{}, event.client_id);

                // Send the new client to other connected clients
                for (h2o::ClientID client_id : m_server.client_ids())
                {
                    if (client_id != event.client_id)
                    {
                        m_server.send_message(client_id,
                            h2o::net_msg::PlayerJoin { event.client_id, h2o::Transform{} });
                    }
                }

                // Send existing clients to the client who just joined
                for (h2o::ClientID client_id : m_server.client_ids())
                {
                    if (client_id == event.client_id)
                        continue;

                    if (auto player = m_scene->get_actor(client_id))
                    {
                        m_server.send_message(event.client_id,
                            h2o::net_msg::PlayerJoin { client_id, player->transform });
                    }
                }
            }
        );

        m_server.handle_message<h2o::net_msg::TransformUpdate>(m_on_received_transform_update_handle,
            [&](h2o::ClientID sender_id, const h2o::net_msg::TransformUpdate& transform_update)
            {
                assert(m_scene);

                auto actor = m_scene->get_actor(sender_id);
                if (!actor)
                    return;

                actor->transform = transform_update.transform;

                for (h2o::ClientID id : m_server.client_ids())
                {
                    if (sender_id != id)
                    {
                        m_server.send_message(id,
                            h2o::net_msg::TransformUpdate { sender_id, actor->transform });
                    }
                }
            }
        );

        return true;
    }

    void BlueVoxelServerModule::cleanup()
    {
        m_chunk_server->stop();
        m_chunk_server.reset();

        m_server.stop(true);
    }

    std::vector<std::type_index> bluevoxel::BlueVoxelServerModule::dependencies() const
    {
        return {
            typeid(h2o::NetworkingModule),
            typeid(h2o::VoxelServerModule)
        };
    }
}