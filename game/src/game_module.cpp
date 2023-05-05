#include "game_module.h"

#include "core/engine.h"
#include "scene_rendering/rendering_scene_system.h"
#include "scene_rendering/mesh_renderer_component.h"
#include "input/input_module.h"
#include "scene/scene_module.h"
#include "scene/scene_headers.h"
#include "game_framework/actors/fps_character_actor.h"
#include "scene_rendering/scene_rendering_module.h"

#include <imgui.h>

namespace game
{
    bool GameModule::init(h2o::Engine& engine)
    {
        // Input setup
        auto input_module = engine.get_module<h2o::InputModule>();
        assert(input_module);
        input_module->register_axis("move_x", h2o::Key::A, h2o::Key::D);
        input_module->register_axis("move_y", h2o::Key::S, h2o::Key::W);
        input_module->register_axis("fly", h2o::Key::LeftControl, h2o::Key::Space);
        input_module->register_axis("cam_x", h2o::MouseDelta::Y, 0.2f, true);
        input_module->register_axis("cam_y", h2o::MouseDelta::X, 0.2f, false);

        m_scene = h2o::Scene::create(engine, "TestGameScene");
        m_scene->add_system<h2o::RenderingSceneSystem>();

        auto player = m_scene->spawn_actor<h2o::FpsCharacterActor>("Player");
        player->transform.position = { -1.0f, 0.0f, 0.0f };
        player->transform.rotation = { 0.0f, 0.0f, 90.0f };

        auto triangle = m_scene->spawn_actor("Triangle");
        triangle->add_component<h2o::MeshRendererComponent>();
        triangle->transform.position = { 0.0f, 0.0f, 0.0f };

        set_tick_phases(h2o::Update);

        return true;
    }

    std::vector<std::type_index> GameModule::dependencies() const
    {
        return { typeid(h2o::InputModule),
                 typeid(h2o::SceneModule),
                 typeid(h2o::SceneRenderingModule) };
    }

    void GameModule::update(f32 delta_time)
    {
        ImGui::Text("TESTTT");
    }
}