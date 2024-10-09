#include "physics/scene/physics_system.h"

namespace h2o
{
    PhysicsSystem::PhysicsSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {
        set_tick_phases(TickPhase::Update);
    }

    void PhysicsSystem::register_mobile_collider(ColliderComponent& collider)
    {
        m_mobile_colliders.push_back(&collider);
    }

    void PhysicsSystem::unregister_mobile_collider(const ColliderComponent& collider)
    {
        erase_if(m_mobile_colliders,
            [&](const ColliderComponent* other)
            {
                return &collider == other;
            }
        );
    }

    void PhysicsSystem::update(f32 delta_time)
    {
        for (const auto collider : m_mobile_colliders)
        {
            assert(collider);

            std::vector<physics::Collider_AABB> near_colliders{};
            const TestingCollisionEvent event{ *collider, near_colliders };

            // This event call fills the near_colliders array
            on_testing_collisions.broadcast(event);

            bool any_collision = false;
            for (const auto& near_collider : near_colliders)
                any_collision = any_collision || collider->collider.resolve(near_collider);

            if (!any_collision)
            {
                collider->collider.position += collider->collider.velocity;
            }
        }
    }
}
