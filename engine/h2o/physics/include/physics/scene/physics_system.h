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
        const physics::Collider_AABB& collider_to_test;

        // A vector of colliders that have a chance of colliding with the tested collider
        std::vector<physics::Collider_AABB>& near_colliders;
    };

    class PhysicsSystem : public SceneSystem
    {
    public:

        explicit PhysicsSystem(const SceneSystemInitializer& system_initializer);
        ~PhysicsSystem() override = default;

        // Returns new collider's id
        u32 register_mobile_collider(ColliderComponent& collider_comp);
        void update_mobile_collider(u32 collider_id);
        void unregister_mobile_collider(u32 collider_id);

        // Environment classes should bind to this to provide colliders to check for collision.
        Event<TestingCollisionEvent> on_testing_collisions{};

    protected:

        void post_update(f32 delta_time) override;

    private:

        struct MobileCollider
        {
            physics::Collider_AABB collider{};
            physics::Collider_AABB previous_collider{};
            ColliderComponent* collider_comp = nullptr;
        };

        std::vector< std::optional<MobileCollider> > m_mobile_colliders{};

    };
}
