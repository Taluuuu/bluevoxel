#include "game_module.h"

#include "core/engine.h"
#include "rendering/camera.h"
#include "rendering/pipeline.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/vertex_array.h"
#include "rendering/buffer.h"
#include "rendering/scene/rendering_scene_system.h"
#include "rendering/scene/mesh_renderer_component.h"
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

        m_scene->spawn_actor<h2o::FpsCharacterActor>("Player");

        auto triangle = m_scene->spawn_actor("Triangle");
        triangle->add_component<h2o::MeshRendererComponent>();

        return true;
    }

    std::vector<std::type_index> GameModule::dependencies() const
    {
        return { typeid(h2o::RenderingModule),
                 typeid(h2o::InputModule),
                 typeid(h2o::SceneModule) };
    }

    void GameModule::tick(h2o::TickPhase phase, f64 delta_time)
    {
    }
}