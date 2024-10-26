#include "world_gen_editor_workspace.h"

#include "core/engine.h"
#include "game_framework/actors/fps_character_actor.h"
#include "input/input_module.h"
#include "scene/scene.h"
#include "scene/scene_networking_system.h"
#include "scene_rendering/rendering_scene_system.h"
#include "ui/imgui.h"
#include "voxel_client/chunk_client.h"
#include "voxel_server/chunk_server.h"
#include "voxel/chunk_generators/chunk_generator_terrain.h"

namespace bluevoxel
{
    WorldGenEditorWorkspace::WorldGenEditorWorkspace(h2o::Tickable* owner)
        : h2o::Tickable(owner)
    {
        m_scene = std::make_shared<h2o::Scene>("client_scene", &m_local_net_peer);
        m_scene->add_system<h2o::RenderingSystem>();
        m_scene->add_system<h2o::ChunkClient, h2o::INetPeer&>(m_local_net_peer);
        m_scene->add_system<h2o::SceneNetworkingSystem, h2o::INetPeer&>(m_local_net_peer);

        const auto chunk_generator = std::make_shared<h2o::ChunkGenerator_Terrain>();
        const auto chunk_server = m_scene->add_system<h2o::ChunkServer, h2o::INetPeer&>(m_local_net_peer, chunk_generator);

        const auto player = m_scene->spawn_actor<h2o::FpsCharacterActor>();
        player->tag_actor(h2o::ActorTag::LocalPlayer);
        player->transform.position = { 0.0f, 200.0f, 0.0f };
        player->fly = true;

        m_point_graph.points.emplace_back(0.0f, 0.0f);
        m_point_graph.points.emplace_back(0.5f, 0.5f);
        m_point_graph.points.emplace_back(1.0f, 0.75f);

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

        if (ImGui::Begin("World Gen Editor"))
        {
            auto* draw_list = ImGui::GetWindowDrawList();

            const v2 cursor_pos = ImGui::GetCursorScreenPos();

            const v2 graph_size{ 250.0f, 250.0f };
            draw_list->AddRectFilled(
                cursor_pos,
                cursor_pos + graph_size,
                ImColor(0.2f, 0.2f, 0.2f, 1.0f));

            for (i32 i = 0; i < m_point_graph.points.size(); i++)
            {
                v2 point = m_point_graph.points[i];
                point.y = 1.0f - point.y;
                const v2 point_on_graph = cursor_pos + point * graph_size;

                draw_list->AddCircleFilled(point_on_graph, 6.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));

                if (i + 1 < m_point_graph.points.size())
                {
                    v2 next_point = m_point_graph.points[i + 1];
                    next_point.y = 1.0f - next_point.y;
                    const v2 next_point_on_graph = cursor_pos + next_point * graph_size;

                    draw_list->AddLine(point_on_graph, next_point_on_graph, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
                }
            }

            ImGui::InvisibleButton("Gen Threshold Graph", graph_size);

            const v2 button_size{ graph_size.x / 2.0f - ImGui::GetStyle().FramePadding.x, 30.0f };
            ImGui::Button("Add Point", button_size);
            ImGui::SameLine();
            ImGui::Button("Remove Point", button_size);
        }
        ImGui::End();
    }
}