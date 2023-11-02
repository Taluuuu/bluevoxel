#include "scene/scene_module.h"

#include <algorithm>

#include "core/engine.h"
#include "scene/scene.h"

namespace h2o
{
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