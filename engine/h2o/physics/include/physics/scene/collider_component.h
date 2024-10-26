#pragma once

#include "core/events.h"
#include "physics/collider_aabb.h"
#include "scene/component.h"

namespace h2o
{
    class PhysicsSystem;

    struct CollisionEvent { v3 displacement{}, normal{}; };

    class ColliderComponent : public Component
    {
    public:

        explicit ColliderComponent(const ComponentInitializer& component_initializer);
        ~ColliderComponent() override;

        void on_displaced(const v3& displacement, const v3& normal);

        void set_size(const v3& size);
        void set_offset(const v3& offset);

        [[nodiscard]] const v3& size() const { return m_size; }
        [[nodiscard]] const v3& offset() const { return m_offset; }

        [[nodiscard]] physics::Collider_AABB calc_collider() const;

        // Tickable interface
        void set_enabled(bool enabled) override;
        void update(f32 delta_time) override;

    public:

        Event<CollisionEvent> on_collision{};

    private:

        v3 m_offset{};
        v3 m_size{};

        v3 m_previous_location{};

        WeakHandle<PhysicsSystem> m_physics_system = nullptr;
        u32 m_collider_id = 0;

    };
}
