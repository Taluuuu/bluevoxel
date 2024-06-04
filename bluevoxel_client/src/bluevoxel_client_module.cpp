#include "bluevoxel_client_module.h"

#include "core/engine.h"
#include "game_framework/actors/fps_character_actor.h"
#include "input/input_module.h"
#include "inventory/inventory_component.h"
#include "rendering/mesh.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture.h"
#include "scene/scene.h"
#include "scene/scene_module.h"
#include "scene/scene_networking_system.h"
#include "scene/scene_net_messages.h"
#include "scene_rendering/mesh_renderer_component.h"
#include "scene_rendering/rendering_scene_system.h"
#include "ui/ui_module.h"
#include "voxel/voxel_pack.h"
#include "voxel_client/chunk_client.h"
#include "voxel_client/block_placing_component.h"

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
        m_input_module->register_axis("cam_x", h2o::MouseMoveDelta::Y, 0.2f, true);
        m_input_module->register_axis("cam_y", h2o::MouseMoveDelta::X, 0.2f, false);

        m_client.handle_message<h2o::net_msg::PlayerJoin>(m_on_client_connected_to_server_handle,
            [this](h2o::PeerID client_id, const h2o::net_msg::PlayerJoin& player_join_event)
            {
                const auto& [actor_id, transform] = player_join_event;
                spawn_remote_player(actor_id, transform);
            }
        );

        m_client.on_connected_to_server.add_listener(m_on_connected_handle,
            [this](const h2o::Client::ConnectionEvent&)
            {
                create_scene();
            }
        );

        m_client.on_disconnected_from_server.add_listener(m_on_disconnected_handle,
            [this](const h2o::Client::ConnectionEvent&)
            {
                m_scene = nullptr;
            }
        );

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
        if (m_input_module->key_state(h2o::Key::Escape).pressed_this_frame)
            m_input_module->set_mouse_state(h2o::MouseCapturePriority::Camera, !m_input_module->is_mouse_captured());

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

    void BlueVoxelClientModule::create_scene()
    {
        assert(m_scene == nullptr);

        m_scene = std::make_shared<h2o::Scene>("client_scene", &m_client);
        m_scene->add_system<h2o::RenderingSystem>();
        m_chunk_client = m_scene->add_system<h2o::ChunkClient, h2o::Client&>(m_client);
        m_scene->add_system<h2o::SceneNetworkingSystem, h2o::Client&>(m_client);

        spawn_local_player();
    }

    void BlueVoxelClientModule::spawn_local_player()
    {
        assert(m_scene);

        auto player = m_scene->spawn_actor<h2o::FpsCharacterActor>();
        player->tag_actor(h2o::ActorTag::LocalPlayer);

        const auto inventory_comp = player->add_component<h2o::InventoryComponent<h2o::Block>>(h2o::Inventory<h2o::Block>{ 5, std::nullopt });
        inventory_comp->inventory().add_item_stack({ .item = h2o::Block{ 1 }, .count = 69 });
        inventory_comp->inventory().add_item_stack({ .item = h2o::Block{ 2 }, .count = 1 });
//        inventory_comp->inventory().add_item_stack({ .item = h2o::Block{ 3 }, .count = 1});
//        inventory_comp->inventory().add_item_stack({ .item = h2o::Block{ 4 }, .count = 1});
//        inventory_comp->inventory().add_item_stack({ .item = h2o::Block{ 5 }, .count = 1});
        m_inventory_ui.emplace(this);
        m_inventory_ui->open(inventory_comp->inventory());

        player->add_component<h2o::BlockPlacingComponent>();
        player->set_replicate_transform(true);
        player->transform.position = { 0.0f, 200.0f, 0.0f };
        player->transform.rotation = { 0.0f, 0.0f, 90.0f };
        player->transform.scale = { 0.5f, 0.5f, 0.5f };
        player->move_speed = 10.0f;
    }

    void BlueVoxelClientModule::spawn_remote_player(h2o::ActorID actor_id, const h2o::Transform& spawn_transform)
    {
        assert(g_engine);

        if (auto remote_player = m_scene->spawn_actor(spawn_transform, actor_id))
        {
            auto mesh_renderer = remote_player->add_component<h2o::MeshRendererComponent>();
            mesh_renderer->set_mesh(
                g_engine->resource_mgr().fetch<h2o::gfx::Mesh>("bluevoxel/models/robot.fbx"));
            mesh_renderer->set_texture(
                g_engine->resource_mgr().fetch<h2o::gfx::Texture>("bluevoxel/textures/robot.png"));
        }
    }
}