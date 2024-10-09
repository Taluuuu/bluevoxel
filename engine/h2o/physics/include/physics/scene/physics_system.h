#pragma once

#include "collider_component.h"
#include "core/events.h"
#include "scene/scene_system.h"

#include <vector>

namespace h2o
{
    class ColliderComponent;

    struct TestingCollisionEvent
    {
        const ColliderComponent& collider_to_test;

        // A vector of colliders that have a chance of colliding with the tested collider
        std::vector<physics::Collider_AABB>& near_colliders;
    };

    class PhysicsSystem : public SceneSystem
    {
    public:

        explicit PhysicsSystem(const SceneSystemInitializer& system_initializer);
        ~PhysicsSystem() override = default;

        void register_mobile_collider(ColliderComponent& collider);
        void unregister_mobile_collider(const ColliderComponent& collider);

        // Environment classes should bind to this to provide colliders to check for
        // collision.
        Event<TestingCollisionEvent> on_testing_collisions{};

    protected:

        void update(f32 delta_time) override;

    private:

        std::vector<ColliderComponent*> m_mobile_colliders{};

    };
}
