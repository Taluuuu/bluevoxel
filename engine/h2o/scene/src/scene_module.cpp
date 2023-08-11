#include "scene/scene_module.h"

#include <algorithm>

#include "core/engine.h"
#include "scene/scene.h"

namespace h2o
{
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