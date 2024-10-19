#pragma once

#include "physics/collider_aabb.h"
#include "scene/component.h"

namespace h2o
{
    class PhysicsSystem;

    class ColliderComponent : public Component
    {
    public:

        explicit ColliderComponent(const ComponentInitializer& component_initializer);
        ~ColliderComponent() override;

        void update(f32 delta_time) override;

        void on_displaced(const v3& displacement);

        void set_size(const v3& size);
        void set_offset(const v3& offset);

        [[nodiscard]] physics::Collider_AABB calc_collider() const;

    private:

        v3 m_offset{};
        v3 m_size{};

        v3 m_previous_location{};

        WeakHandle<PhysicsSystem> m_physics_system = nullptr;
        u32 m_collider_id = 0;

    };
}
