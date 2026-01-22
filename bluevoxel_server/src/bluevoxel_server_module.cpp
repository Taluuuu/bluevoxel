#include "bluevoxel_server_module.h"

#include "core/engine.h"
#include "networking/networking_module.h"
#include "networking/networking_utils.h"
#include "scene/player.h"
#include "scene/scene.h"
#include "scene/scene_net_messages.h"
#include "scene/scene_networking_components.h"
#include "scene/scene_module.h"
#include "voxel/chunk_generators/chunk_generator_flat.h"
#include "voxel/chunk_generators/chunk_generator_terrain.h"
#include "voxel/voxel_pack.h"
#include "voxel_server/chunk_server.h"
#include "weather/weather_system.h"

#include <vector>

namespace bluevoxel
{
    BlueVoxelServerModule::BlueVoxelServerModule()
        : m_server(g_engine)
    {}

    bool BlueVoxelServerModule::init(h2o::Engine& engine)
    {
        auto voxel_pack = engine
            .resource_mgr()
            .fetch<h2o::VoxelPack>("bluevoxel/voxel/");

        if (!voxel_pack)
            return false;

        auto& voxel_module = engine.get_module_checked<h2o::VoxelModule>();
        voxel_module.set_voxel_pack(voxel_pack);

        if (!m_server.start(1338))
            return false;

        // Setup scene
        m_scene = std::make_shared<h2o::Scene>("server_scene", &m_server);

        m_chunk_server = m_scene->add_system<h2o::ChunkServer, h2o::Server&>(m_server);
        m_scene->add_system<h2o::WeatherSystem>();

        m_server.on_player_joined.add_listener(m_player_joined_event_handle,
            [&](const h2o::Server::PlayerConnectionChangedEvent& event)
            {
                auto& registry = m_scene->registry();

                const auto player_entity = registry.create();
                registry.emplace<h2o::Position>(player_entity);
                registry.emplace<h2o::Rotation>(player_entity);
                registry.emplace<h2o::Scale>(player_entity);
                registry.emplace<h2o::Velocity>(player_entity);
                registry.emplace<h2o::Player>(player_entity);
                registry.emplace<h2o::NetworkSync>(player_entity);
            }
        );

        // m_server.on_player_left.add_listener(m_player_left_event_handle,
        //     [&](const h2o::Server::PlayerConnectionChangedEvent& event)
        //     {
        //
        //     }
        // );

        return true;
    }

    void BlueVoxelServerModule::cleanup()
    {
        m_chunk_server.reset();
        m_server.stop();
    }

    std::vector<std::type_index> BlueVoxelServerModule::dependencies() const
    {
        return {
            typeid(h2o::NetworkingModule),
            typeid(h2o::SceneModule),
            typeid(h2o::VoxelModule)
        };
    }
}
