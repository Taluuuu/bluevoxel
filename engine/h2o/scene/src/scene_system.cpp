#include "scene/scene_system.h"

#include <cassert>

namespace h2o
{
    SceneSystem::SceneSystem(const SceneSystemInitializer& system_initializer)
        : m_scene(&system_initializer.owning_scene)
    {}
}