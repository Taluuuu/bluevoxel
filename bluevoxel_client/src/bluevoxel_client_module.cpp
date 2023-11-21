#include "bluevoxel_client_module.h"

#include "core/engine.h"
#include "game_framework/actors/fps_character_actor.h"
#include "input/input_module.h"
#include "networking/test_message.h"
#include "rendering/mesh.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "scene/scene_module.h"
#include "scene/scene_networking_system.h"
#include "scene/scene_net_messages.h"
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
    BlueVoxelClientModule::BlueVoxelClientModule()
        : Tickable(g_engine)
        , m_client(g_engine)
    {}

    bool BlueVoxelClientModule::init(h2o::Engine& engine)
    {
        set_tick_phases(h2o::TickPhase::Update);

        // Input setup
        auto input_module = engine.get_module<h2o::InputModule>();
        assert(input_module);
        input_module->register_axis("move_x", h2o::Key::A, h2o::Key::D);
        input_module->register_axis("move_y", h2o::Key::S, h2o::Key::W);
        input_module->register_axis("fly", h2o::Key::LeftControl, h2o::Key::Space);
        input_module->register_axis("cam_x", h2o::MouseDelta::Y, 0.2f, true);
        input_module->register_axis("cam_y", h2o::MouseDelta::X, 0.2f, false);

        m_scene = std::make_shared<h2o::Scene>("client_scene", &m_client);
        m_scene->add_system<h2o::RenderingSystem>();
        m_chunk_client = m_scene->add_system<h2o::ChunkClient, h2o::Client&>(m_client);
        m_scene->add_system<h2o::SceneNetworkingSystem, h2o::Client&>(m_client);

        spawn_local_player();

        m_client.handle_message<h2o::net_msg::PlayerJoin>(m_on_client_connected_to_server_handle,
            [&](h2o::PeerID client_id, const h2o::net_msg::PlayerJoin& player_join_event)
            {
                const auto& [actor_id, transform] = player_join_event;
                spawn_remote_player(actor_id, transform);
            }
        );

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

            if (ImGui::Button("Disconnect from Server"))
                m_client.stop(true);

            ImGui::End();

            break;
        }
        }

        if (m_chunk_client)
        {
            ImGui::Begin("Voxel");

            ImGui::SliderFloat3("Light Direction", &m_chunk_client->light_dir.x, -1.0f, 1.0f);
            ImGui::ColorEdit3("Light Color", &m_chunk_client->light_color.x);
            ImGui::SliderFloat("Ambient Strength", &m_chunk_client->ambient_strength, 0.0f, 1.0f);

            ImGui::End();
        }
    }

    void BlueVoxelClientModule::spawn_local_player()
    {
        assert(m_scene);

        auto player = m_scene->spawn_actor<h2o::FpsCharacterActor>();
        player->tag_actor(h2o::ActorTag::LocalPlayer);
        player->add_component<h2o::BlockPlacingComponent>();
        player->set_replicate_transform(true);
        player->transform.position = { 0.0f, 150.0f, 0.0f };
        player->transform.rotation = { 0.0f, 0.0f, 90.0f };
        player->transform.scale = { 0.5f, 0.5f, 0.5f };
        player->move_speed = 100.0f;
    }

    void BlueVoxelClientModule::spawn_remote_player(h2o::ActorID actor_id, const h2o::Transform& spawn_transform)
    {
        assert(g_engine);

        auto rendering_module = g_engine->get_module<h2o::RenderingModule>();
        assert(rendering_module);

        if (auto remote_player = m_scene->spawn_actor(spawn_transform, actor_id))
        {
            auto mesh_renderer = remote_player->add_component<h2o::MeshRendererComponent>();
            mesh_renderer->set_mesh(
                g_engine->resource_mgr().fetch<h2o::gfx::Mesh>("../Resources/bluevoxel_client/models/robot.fbx"));
            mesh_renderer->set_texture(
                rendering_module->renderer().fetch_or_load_texture("../Resources/bluevoxel_client/textures/robot.png"));
        }
    }
}