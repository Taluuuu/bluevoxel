#include "scene/scene_ecs.h"

#include "core/engine.h"

namespace h2o
{
    Scene_ECS::Scene_ECS()
        : Tickable(g_engine)
    {
    }

    Entity Scene_ECS::spawn_entity(const Entity& prefab)
    {
        if (prefab)
        {
            // m_ecs.observer(prefab).event
            // flecs::event
        }

        return flecs::entity::null();
    }

    void Scene_ECS::destroy_entity(const Entity& entity)
    {
        entity.destruct();
    }
}
