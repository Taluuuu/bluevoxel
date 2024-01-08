#include "bluevoxel_client_module.h"

#include "core/engine.h"
#include "game_framework/actors/fps_character_actor.h"
#include "input/input_module.h"
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
#include "scene_rendering/scene_rendering_module.h"
#include "ui/ui_module.h"
#include "ui/ui_renderer.h"
#include "voxel/voxel_pack.h"
#include "voxel_client/chunk_client.h"
#include "voxel_client/block_placing_component.h"
#include "voxel_client/voxel_client_module.h"

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
            .fetch<h2o::VoxelPack>("../Resources/bluevoxel/voxel/");

        if (!voxel_pack)
            return false;

        auto& voxel_module = engine.get_module_checked<h2o::VoxelModule>();
        voxel_module.set_voxel_pack(voxel_pack);

        set_tick_phases(h2o::TickPhase::Update);

        // Input setup
        auto& input_module = engine.get_module_checked<h2o::InputModule>();
        input_module.register_axis("move_x", h2o::Key::A, h2o::Key::D);
        input_module.register_axis("move_y", h2o::Key::S, h2o::Key::W);
        input_module.register_axis("fly", h2o::Key::LeftControl, h2o::Key::Space);
        input_module.register_axis("cam_x", h2o::MouseDelta::Y, 0.2f, true);
        input_module.register_axis("cam_y", h2o::MouseDelta::X, 0.2f, false);

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

        m_ui_module = &engine.get_module_checked<h2o::UIModule>();

        return true;
    }

    void BlueVoxelClientModule::cleanup()
    {
        m_client.stop();
    }

    std::vector<std::type_index> BlueVoxelClientModule::dependencies() const
    {
        return {
            typeid(h2o::VoxelClientModule),
            typeid(h2o::SceneModule),
            typeid(h2o::SceneRenderingModule),
            typeid(h2o::RenderingModule),
            typeid(h2o::UIModule)
        };
    }

    void BlueVoxelClientModule::update(f32 delta_time)
    {
        m_ui_module->window("Connection", { { 50.0f, 50.0f }, { 250.0f, 250.0f } },
            [&](h2o::IUIRenderer& ui)
            {
                switch (m_client.connection_state())
                {
                case h2o::ConnectionState::Disconnected:
                {
                    ui.row(25.0f, 1);

                    ui.input_text("Server IP", m_server_ip);

                    if (ui.input_int("Server Port", m_server_port))
                        m_server_port = glm::clamp(m_server_port, 0, 65'535);

                    if (ui.button("Connect"))
                        m_client.connect(m_server_ip, m_server_port);

                    break;
                }

                case h2o::ConnectionState::Connecting:
                {
                    ui.row(25.0f, 1);
                    ui.label("Connecting to Server...");

                    break;
                }

                case h2o::ConnectionState::Connected:
                {
                    ui.row(25.0f, 1);
                    if (ui.button("Disconnect"))
                        m_client.stop();

                    break;
                }
                }
            }
        );
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
        player->add_component<h2o::BlockPlacingComponent>();
        player->set_replicate_transform(true);
        player->transform.position = { 0.0f, 300.0f, 0.0f };
        player->transform.rotation = { 0.0f, 0.0f, 90.0f };
        player->transform.scale = { 0.5f, 0.5f, 0.5f };
        player->move_speed = 100.0f;
    }

    void BlueVoxelClientModule::spawn_remote_player(h2o::ActorID actor_id, const h2o::Transform& spawn_transform)
    {
        assert(g_engine);

        if (auto remote_player = m_scene->spawn_actor(spawn_transform, actor_id))
        {
            auto mesh_renderer = remote_player->add_component<h2o::MeshRendererComponent>();
            mesh_renderer->set_mesh(
                g_engine->resource_mgr().fetch<h2o::gfx::Mesh>("../Resources/bluevoxel/models/robot.fbx"));
            mesh_renderer->set_texture(
                g_engine->resource_mgr().fetch<h2o::gfx::Texture>("../Resources/bluevoxel/textures/robot.png"));
        }
    }
}