#include "bluevoxel_client_module.h"

#include "core/engine.h"
#include "game_framework/actors/player_character.h"
#include "game_framework/components/fps_camera_component.h"
#include "game_framework/components/player_movement_component.h"
#include "game_framework/systems/fps_camera_system.h"
#include "game_framework/systems/player_movement_system.h"
#include "input/input_module.h"
#include "physics/scene/physics_system.h"
#include "rendering/mesh.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture.h"
#include "scene/player.h"
#include "scene/scene.h"
#include "scene/scene_module.h"
#include "scene/scene_networking_system.h"
#include "scene/scene_net_messages.h"
#include "scene_rendering/camera_component.h"
#include "scene_rendering/mesh_renderer_component.h"
#include "scene_rendering/rendering_scene_system.h"
#include "ui/ui_module.h"
#include "voxel/voxel_pack.h"
#include "voxel_client/chunk_client.h"
#include "voxel_client/block_placing_component.h"
#include "weather/weather_system.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace bluevoxel
{
    BlueVoxelClientModule::BlueVoxelClientModule()
        : Tickable(g_engine)
        , m_client(this)
    {}

    bool BlueVoxelClientModule::init(h2o::Engine& engine)
    {
        auto voxel_pack = engine
            .resource_mgr()
            .fetch<h2o::VoxelPack>("bluevoxel/voxel/");

        if (!voxel_pack)
            return false;

        auto& voxel_module = engine.get_module_checked<h2o::VoxelModule>();
        voxel_module.set_voxel_pack(voxel_pack);

        set_tick_phases(h2o::TickPhase::Update);

        // Input setup
        m_input_module = &engine.get_module_checked<h2o::InputModule>();
        m_input_module->register_axis("move_x", h2o::Key::A, h2o::Key::D);
        m_input_module->register_axis("move_y", h2o::Key::S, h2o::Key::W);
        m_input_module->register_axis("fly", h2o::Key::LeftControl, h2o::Key::Space);
        m_input_module->register_axis("cam_x", h2o::MouseMoveDelta::Y, 0.001f, true);
        m_input_module->register_axis("cam_y", h2o::MouseMoveDelta::X, 0.001f, false);

        m_client.on_connected_to_server.add_listener(m_on_connected_handle,
            [this](const h2o::Client::ConnectionEvent&)
            {
                g_engine->layer_stack().pop_layer(h2o::Layer::PauseMenu);
                create_scene();
            }
        );

        m_client.on_disconnected_from_server.add_listener(m_on_disconnected_handle,
            [this](const h2o::Client::ConnectionEvent&)
            {
                m_scene = nullptr;
            }
        );

        g_engine->layer_stack().push_layer(h2o::Layer::PauseMenu, { false, true });

        return true;
    }

    void BlueVoxelClientModule::cleanup()
    {
        m_client.stop();
    }

    std::vector<std::type_index> BlueVoxelClientModule::dependencies() const
    {
        return {
            typeid(h2o::SceneModule),
            typeid(h2o::RenderingModule),
            typeid(h2o::UIModule),
            typeid(h2o::VoxelModule),
        };
    }

    void BlueVoxelClientModule::update(f32 delta_time)
    {
        auto& layer_stack = g_engine->layer_stack();
        if (m_input_module->key_state(h2o::Key::Escape).pressed_this_frame)
        {
            if (layer_stack.top_layer() == h2o::Layer::PauseMenu)
            {
                layer_stack.pop_layer(h2o::Layer::PauseMenu);
            }
            else
            {
                layer_stack.push_layer(h2o::Layer::PauseMenu,
                    h2o::LayerData{ false, true });
            }
        }

        if (layer_stack.top_layer() == h2o::Layer::PauseMenu)
        {
            ImGui::Begin("Connection");

            switch (m_client.connection_state())
            {
            case h2o::ConnectionState::Disconnected:
            {
                ImGui::InputText("Server IP", &m_server_ip);

                if (ImGui::InputInt("Server Port", &m_server_port))
                    m_server_port = glm::clamp(m_server_port, 0, 65'535);

                if (ImGui::Button("Connect"))
                    m_client.connect(m_server_ip, m_server_port);

                break;
            }

            case h2o::ConnectionState::Connecting:
            {
                ImGui::Text("Connecting to Server...");

                break;
            }

            case h2o::ConnectionState::Connected:
            {
                if (ImGui::Button("Disconnect"))
                    m_client.stop();

                break;
            }
            }

            ImGui::End();
        }
    }

    void BlueVoxelClientModule::create_scene()
    {
        assert(m_scene == nullptr);

        m_scene = std::make_shared<h2o::Scene>("client_scene", &m_client);
        m_scene->add_system<h2o::PhysicsSystem>();
        m_scene->add_system<h2o::RenderingSystem>();
        m_scene->add_system<h2o::SceneNetworkingSystem, h2o::Client&>(m_client);
        m_scene->add_system<h2o::WeatherSystem>();
        m_scene->add_system<h2o::PlayerMovementSystem>();
        m_scene->add_system<h2o::FpsCameraSystem>();
        m_chunk_client = m_scene->add_system<h2o::ChunkClient, h2o::Client&>(m_client);

        m_scene->on_network_sync_entity_created.add_listener(m_on_network_sync_entity_created_handle,
            [this](const entt::entity entity)
            {
                auto& registry = m_scene->registry();
                if (!registry.any_of<h2o::Player>(entity))
                    return;

                // Add some local only components when we spawn a player
                if (const auto network_sync = registry.try_get<h2o::NetworkSync>(entity))
                {
                    if (network_sync->owner == m_scene->local_peer_id())
                    {
                        h2o::log::info("Local owner: {}, Entity ID: {}, actual entity: {}", network_sync->owner, network_sync->entity_id, (u32)entity);

                        if (!registry.any_of<h2o::PlayerMovementComp>(entity))
                            registry.emplace<h2o::PlayerMovementComp>(entity);

                        if (!registry.any_of<h2o::CameraComp>(entity))
                            registry.emplace<h2o::CameraComp>(entity);

                        if (!registry.any_of<h2o::FpsCameraComp>(entity))
                            registry.emplace<h2o::FpsCameraComp>(entity);
                    }
                    else
                    {
                        h2o::log::info("Remote owner: {}, Entity ID: {}, actual entity: {}", network_sync->owner, network_sync->entity_id, (u32)entity);
                    }
                }
            }
        );
    }
}
