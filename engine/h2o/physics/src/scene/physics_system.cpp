#include "physics/scene/physics_system.h"

#include "core/log.h"

namespace h2o
{
    PhysicsSystem::PhysicsSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {
        set_tick_phases(TickPhase::PhysicsUpdate);
    }

    bool PhysicsSystem::raycast(const v3& origin, const v3& direction, const f32 range, RayHit& out_hit) const
    {
        out_hit.distance = range + 0.0001f;
        on_raycasting.broadcast(RaycastingEvent{ origin, direction, range, out_hit });
        
        return out_hit.distance < range;
    }

    bool PhysicsSystem::collides(const physics::Collider_AABB& collider_to_test) const
    {
        std::vector<physics::Collider_AABB> near_colliders{};
        {
            // This event call fills the near_colliders array
            const TestingCollisionEvent event{ collider_to_test, near_colliders };
            on_testing_collisions.broadcast(event);
        }

        for (const auto& near_collider : near_colliders)
        {
            if (collider_to_test.intersects(near_collider))
                return true;
        }

        return false;
    }

    u32 PhysicsSystem::register_mobile_collider(ColliderComponent& collider_comp)
    {
        const physics::Collider_AABB collider = collider_comp.calc_collider();
        const MobileCollider mobile_collider {
            .collider = collider,
            .previous_collider = collider,
            .collider_comp = &collider_comp
        };

        u32 id = 0;
        for (; id < m_mobile_colliders.size(); id++)
        {
            if (auto& collider_data = m_mobile_colliders[id]; !collider_data)
            {
                collider_data = mobile_collider;
                return id;
            }
        }

        m_mobile_colliders.emplace_back(mobile_collider);
        return id;
    }

    void PhysicsSystem::update_mobile_collider(const u32 collider_id)
    {
        if (collider_id < m_mobile_colliders.size())
        {
            auto& collider = m_mobile_colliders[collider_id];
            assert(collider.has_value());
            assert(collider->collider_comp != nullptr);

            collider->previous_collider = collider->collider;
            collider->collider = collider->collider_comp->calc_collider();
        }
    }

    void PhysicsSystem::unregister_mobile_collider(const u32 collider_id)
    {
        if (collider_id < m_mobile_colliders.size())
            m_mobile_colliders[collider_id] = std::nullopt;
    }

    void PhysicsSystem::physics_update(f32 delta_time)
    {
        for (auto& collider_data : m_mobile_colliders)
        {
            assert(collider_data.has_value());

            std::vector<physics::Collider_AABB> near_colliders{};
            {
                // This event call fills the near_colliders array
                const TestingCollisionEvent event{ collider_data->collider, near_colliders };
                on_testing_collisions.broadcast(event);
            }

            for (const auto& other_collider : near_colliders)
            {
                v3 normal;
                auto displacement = physics::Collider_AABB::resolve(
                    collider_data->collider,
                    collider_data->previous_collider,
                    other_collider,
                    normal);

                if (displacement)
                {
                    collider_data->collider.position += *displacement;
                    collider_data->collider_comp->on_displaced(*displacement, normal);
                }
            }
        }
    }
}
