#include "bluevoxel_client_module.h"

#include "core/engine.h"
#include "game_framework/actors/fps_character_actor.h"
#include "input/input_module.h"
#include "networking/test_message.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "scene/scene_module.h"
#include "scene_rendering/mesh_renderer_component.h"
#include "scene_rendering/rendering_scene_system.h"
#include "scene_rendering/scene_rendering_module.h"
#include "voxel_client/chunk_client.h"
#include "voxel_client/block_placing_component.h"
#include "voxel_client/voxel_client_module.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace bluevoxel
{
    bool BlueVoxelClientModule::init(h2o::Engine& engine)
    {
        set_tick_phases(h2o::TickPhase_Update);

        // Input setup
        auto input_module = engine.get_module<h2o::InputModule>();
        assert(input_module);
        input_module->register_axis("move_x", h2o::Key::A, h2o::Key::D);
        input_module->register_axis("move_y", h2o::Key::S, h2o::Key::W);
        input_module->register_axis("fly", h2o::Key::LeftControl, h2o::Key::Space);
        input_module->register_axis("cam_x", h2o::MouseDelta::Y, 0.2f, true);
        input_module->register_axis("cam_y", h2o::MouseDelta::X, 0.2f, false);

        m_scene = h2o::Scene::create(engine, "client_scene");
        m_scene->add_system<h2o::RenderingSystem>();
        m_scene->add_system<h2o::ChunkClient, h2o::Client&>(m_client);

        auto player = m_scene->spawn_actor<h2o::FpsCharacterActor>("player");
        player->add_component<h2o::BlockPlacingComponent>();
        player->transform.position = { 5.0f, 50.0f, 0.0f };
        player->transform.rotation = { 0.0f, 180.0f, 90.0f };
        player->move_speed = 10.0f;

        auto triangle = m_scene->spawn_actor("triangle");
        triangle->add_component<h2o::MeshRendererComponent>();
        triangle->transform.position = { 0.0f, 0.0f, 0.0f };

        return true;
    }

    void BlueVoxelClientModule::cleanup()
    {
        m_client.stop(true);
    }

    std::vector<std::type_index> BlueVoxelClientModule::dependencies() const
    {
        return {
            typeid(h2o::VoxelClientModule),
            typeid(h2o::SceneModule),
            typeid(h2o::SceneRenderingModule),
            typeid(h2o::RenderingModule)
        };
    }

    void BlueVoxelClientModule::update(f32 delta_time)
    {
        switch (m_client.connection_state())
        {
        case h2o::ConnectionState::Disconnected:
        {
            ImGui::Begin("Connect to Server");

            ImGui::InputText("Server IP", &m_server_ip);
            if (ImGui::InputInt("Server Port", &m_server_port, 0))
                m_server_port = glm::clamp(m_server_port, 0, 65'535);

            if (ImGui::Button("Connect to server"))
                m_client.connect(m_server_ip, m_server_port);

            ImGui::End();
            break;
        }

        case h2o::ConnectionState::Connecting:
        {
            ImGui::Begin("Connection");

            ImGui::Text("Connecting...");

            ImGui::End();
            break;
        }

        case h2o::ConnectionState::Connected:
        {
            ImGui::Begin("Connection");

            if (ImGui::Button("Send packet"))
            {
                h2o::TestMessage msg { "texte :))", 43843 };
                m_client.send_message(0, msg);
            }

            if (ImGui::Button("Disconnect from Server"))
                m_client.stop(true);

            ImGui::End();
            break;
        }
        }
    }
}