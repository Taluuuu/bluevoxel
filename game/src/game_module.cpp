#include "game_module.h"

#include "core/engine.h"
#include "scene_rendering/rendering_scene_system.h"
#include "scene_rendering/mesh_renderer_component.h"
#include "input/input_module.h"
#include "scene/scene_module.h"
#include "scene/scene_headers.h"
#include "game_framework/actors/fps_character_actor.h"

namespace game
{
    bool GameModule::init(h2o::Engine& engine)
    {
        engine.register_tickable(this, h2o::TickPhase::Render | h2o::TickPhase::Update);

        auto input_module = engine.get_module<h2o::InputModule>();
        assert(input_module);
        input_module->register_axis("move_x", h2o::Key::A, h2o::Key::D);
        input_module->register_axis("move_y", h2o::Key::S, h2o::Key::W);

        m_scene = h2o::Scene::create(engine, "TestGameScene");
        m_scene->add_system<h2o::RenderingSceneSystem>();

        auto player = m_scene->spawn_actor<h2o::FpsCharacterActor>("Player");
        player->transform.position = { -5.0f, 0.0f, 0.0f };
        player->transform.rotation = { 0.0f, 0.0f, 0.0f };

        auto triangle = m_scene->spawn_actor("Triangle");
        triangle->add_component<h2o::MeshRendererComponent>();
        triangle->transform.position = { 0.0f, 2.0f, 0.0f };

        return true;
    }

    std::vector<std::type_index> GameModule::dependencies() const
    {
        return { typeid(h2o::InputModule),
                 typeid(h2o::SceneModule) };
    }

    void GameModule::tick(h2o::TickPhase phase, f64 delta_time)
    {
    }
}