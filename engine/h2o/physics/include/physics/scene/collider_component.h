#pragma once

#include "physics/collider_aabb.h"
#include "scene/component.h"

namespace h2o
{
    class ColliderComponent : public Component
    {
    public:

        explicit ColliderComponent(const ComponentInitializer& component_initializer);
        ~ColliderComponent() override;

        void update(f32 delta_time) override;

        void on_collision_resolve();

    public:

        physics::Collider_AABB collider{};

    };
}
