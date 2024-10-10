#pragma once

#include "core/tickable.h"

#include <flecs.h>

namespace h2o
{
    using Entity = flecs::entity;

    class Scene_ECS : public Tickable
    {
    public:

        Scene_ECS();
        Scene_ECS(const Scene_ECS&) = delete;

        Entity spawn_entity(const Entity& prefab = flecs::entity::null());
        void destroy_entity(const Entity& entity);

        [[nodiscard]] flecs::world& ecs() { return m_ecs; }
        [[nodiscard]] const flecs::world& ecs() const { return m_ecs; }

    private:

        flecs::world m_ecs{};

    };
}
