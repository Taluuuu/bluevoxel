#include "world_gen_editor_workspace.h"

#include "core/engine.h"
#include "game_framework/actors/player_character.h"
#include "game_framework/components/player_movement_component.h"
#include "graph/graph_ui.h"
#include "input/input_module.h"
#include "scene/scene.h"
#include "scene/scene_networking_system.h"
#include "scene_rendering/rendering_scene_system.h"
#include "ui/imgui.h"
#include "voxel/chunk_generators/chunk_generator_terrain.h"
#include "voxel/voxel_pack.h"
#include "voxel_client/chunk_client.h"
#include "voxel_server/chunk_server.h"

namespace bluevoxel
{
    WorldGenEditorWorkspace::WorldGenEditorWorkspace(h2o::Tickable* owner)
        : h2o::Tickable(owner)
    {
        m_scene = std::make_shared<h2o::Scene>("client_scene", &m_local_net_peer);
        m_scene->add_system<h2o::RenderingSystem>();
        m_chunk_client = m_scene->add_system<h2o::ChunkClient, h2o::INetPeer&>(m_local_net_peer);
        m_scene->add_system<h2o::SceneNetworkingSystem, h2o::INetPeer&>(m_local_net_peer);

        m_chunk_server = m_scene->add_system<h2o::ChunkServer, h2o::INetPeer&>(m_local_net_peer);

        const auto player = m_scene->spawn_actor<h2o::PlayerCharacter>();
        player->tag_actor(h2o::ActorTag::LocalPlayer);
        player->transform.position = { 0.0f, 300.0f, 0.0f };
        if (const auto movement_comp = player->get_component<h2o::PlayerMovementComponent>())
        {
            movement_comp->fly = true;
        }

        set_tick_phases(h2o::TickPhase::Update);
    }

    void WorldGenEditorWorkspace::update(f32 delta_time)
    {
        auto& layer_stack = g_engine->layer_stack();
        const auto& input = g_engine->get_module_checked<h2o::InputModule>();
        if (input.key_state(h2o::Key::Escape).pressed_this_frame)
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

        const auto voxel_pack = g_engine->get_module_checked<h2o::VoxelModule>().voxel_pack();
        assert(voxel_pack);

        if (ImGui::Begin("World Gen Editor"))
        {
            if (ImGui::Button("Save"))
                voxel_pack->save();

            if (ImGui::Button("Update"))
                regenerate(false);

            if (ImGui::Button("Regenerate"))
                regenerate(true);

            if (auto terrain_generator = dynamic_cast<h2o::ChunkGenerator_Terrain*>(voxel_pack->chunk_generator().get()))
                h2o::graph::draw_ui(terrain_generator->m_graph, m_graph_ui_context, true);
        }
        ImGui::End();
    }

    void WorldGenEditorWorkspace::regenerate(const bool use_new_seed)
    {
        m_chunk_server->regenerate(use_new_seed);
        m_chunk_client->reload_all();
    }
}
