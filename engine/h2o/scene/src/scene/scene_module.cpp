#include "scene/scene_module.h"

#include "scene/scene.h"

namespace h2o
{
    void SceneModule::on_engine_starts_closing()
    {
        for (auto scene : m_scenes)
            scene->cleanup();
    }

    void SceneModule::register_scene(Scene& scene)
    {
        m_scenes.insert(&scene);
    }

    void SceneModule::unregister_scene(Scene& scene)
    {
        m_scenes.erase(&scene);
    }

    void SceneModule::set_current_scene(Scene* scene)
    {

    }
}
