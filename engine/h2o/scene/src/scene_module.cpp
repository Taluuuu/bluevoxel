#include "scene/scene_module.h"

#include "core/engine.h"
#include "scene/scene.h"

namespace h2o
{
    bool SceneModule::init(Engine& engine)
    {
        engine.register_tickable(this, TickPhase::Update);
        return true;
    }

    void SceneModule::tick(TickPhase phase, f64 delta_time)
    {
        for (auto scene : m_scenes)
            scene->tick(static_cast<f32>(delta_time));
    }

    void SceneModule::register_scene(Scene& scene)
    {
        if (std::find(m_scenes.begin(), m_scenes.end(), &scene) != m_scenes.end())
        {
            // Scene is already registered
            return;
        }

        m_scenes.push_back(&scene);
    }

    void SceneModule::unregister_scene(Scene& scene)
    {
        std::erase(m_scenes, &scene);
    }
}