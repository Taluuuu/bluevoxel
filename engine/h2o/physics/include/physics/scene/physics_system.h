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

    struct RayHit
    {
        f32 distance = std::numeric_limits<f32>::max();
        v3 hit_location{};
    };

    struct RaycastingEvent
    {
        v3 origin{}, direction{};
        f32 range = 10.0f;
        RayHit& hit_result;
    };

    class PhysicsSystem : public SceneSystem
    {
    public:

        explicit PhysicsSystem(const SceneSystemInitializer& system_initializer);
        ~PhysicsSystem() override = default;

        [[nodiscard]] bool raycast(const v3& origin, const v3& direction, f32 range, RayHit& out_hit) const;
        [[nodiscard]] bool collides(const physics::Collider_AABB& collider_to_test) const;

        // Returns new collider's id
        u32 register_mobile_collider(ColliderComponent& collider_comp);
        void update_mobile_collider(u32 collider_id);
        void unregister_mobile_collider(u32 collider_id);

        // Different systems can bind to these to influence physics
        Event<TestingCollisionEvent> on_testing_collisions{};
        Event<RaycastingEvent> on_raycasting{};

    protected:

        void physics_update(f32 delta_time) override;

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
