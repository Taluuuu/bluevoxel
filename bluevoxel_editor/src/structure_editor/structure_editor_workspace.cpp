#include "structure_editor_workspace.h"

#include "core/engine.h"
#include "game_framework/actors/fps_character_actor.h"
#include "game_framework/components/third_person_camera_component.h"
#include "input/input_component.h"
#include "input/input_module.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"

namespace bluevoxel
{
    StructureEditorWorkspace::StructureEditorWorkspace(h2o::Tickable* owner)
        : h2o::Tickable(owner)
        , m_voxel_world_renderer(*this, m_chunk_manager)
    {
        m_chunk_manager.fetch_or_create_chunk_mut(v3i{0},
            [&](h2o::Chunk* chunk)
            {
                chunk->set_block_at(v3i{0}, h2o::Block{1});
            }
        );

        m_chunk_manager.broadcast_events();

        m_scene = std::make_shared<h2o::Scene>("editor_scene", nullptr);
        m_scene->add_system<h2o::RenderingSystem>();
        // m_scene->cleanup();

        // const auto player = m_scene->spawn_actor<h2o::FpsCharacterActor>();
        // player->tag_actor(h2o::ActorTag::LocalPlayer);
        // player->transform.position = { 0.0f, 0.0f, 0.0f };
        // player->transform.rotation = { 0.0f, 0.0f, 0.0f };
        // player->transform.scale = { 0.5f, 0.5f, 0.5f };
        // player->move_speed = 100.0f;

        auto camera = m_scene->spawn_actor();
        camera->add_component<h2o::InputComponent>();
        auto tps_cam = camera->add_component<h2o::ThirdPersonCameraComponent>();
        tps_cam->rotate_mouse_button = h2o::MouseButton::Right;
        camera->tag_actor(h2o::ActorTag::LocalPlayer);
        camera->transform.position = { 0.5f, 0.5f, 0.5f };
        camera->transform.rotation = { 0.0f, 0.0f, 0.0f };

        set_tick_phases(h2o::TickPhase::Update);
    }

    void StructureEditorWorkspace::update(f32 delta_time)
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
    }
}
