#include "scene/scene_system.h"

#include <cassert>

#include "scene/scene.h"

namespace h2o
{
    SceneSystem::SceneSystem(const SceneSystemInitializer& system_initializer)
        : Tickable(&system_initializer.owning_scene)
        , m_scene(&system_initializer.owning_scene)
    {}
}