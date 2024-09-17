#include "world_gen_editor_workspace.h"

#include "core/engine.h"
#include "game_framework/actors/fps_character_actor.h"
#include "input/input_module.h"
#include "scene/scene.h"
#include "scene/scene_networking_system.h"
#include "scene_rendering/rendering_scene_system.h"
#include "voxel/voxel_pack.h"
#include "voxel_client/chunk_client.h"
#include "voxel_server/chunk_server.h"
#include "voxel/chunk_generators/chunk_generator_terrain.h"

namespace bluevoxel
{
    WorldGenEditorWorkspace::WorldGenEditorWorkspace(h2o::Tickable* owner)
        : h2o::Tickable(owner)
        , m_input_module(&g_engine->get_module_checked<h2o::InputModule>())
    {
        m_scene = std::make_shared<h2o::Scene>("client_scene", &m_local_net_peer);
        m_scene->add_system<h2o::RenderingSystem>();
        m_scene->add_system<h2o::ChunkClient, h2o::INetPeer&>(m_local_net_peer);
        m_scene->add_system<h2o::SceneNetworkingSystem, h2o::INetPeer&>(m_local_net_peer);

        auto chunk_generator = std::make_shared<h2o::ChunkGenerator_Terrain>();
        const auto chunk_server = m_scene->add_system<h2o::ChunkServer, h2o::INetPeer&>(m_local_net_peer, chunk_generator);

        const auto player = m_scene->spawn_actor<h2o::FpsCharacterActor>();
        player->tag_actor(h2o::ActorTag::LocalPlayer);
        player->transform.position = { 0.0f, 200.0f, 0.0f };
        player->transform.rotation = { 0.0f, 0.0f, 90.0f };
        player->transform.scale = { 0.5f, 0.5f, 0.5f };
        player->move_speed = 100.0f;

        set_tick_phases(h2o::TickPhase::Update);
    }

    void WorldGenEditorWorkspace::update(f32 delta_time)
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
    }
}