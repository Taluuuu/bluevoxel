#include "physics/scene/collider_component.h"

#include "physics/scene/physics_system.h"
#include "scene/actor.h"
#include "scene/scene.h"

namespace h2o
{
    ColliderComponent::ColliderComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
        , m_previous_location(component_initializer.owner.transform.position)
        , m_physics_system(m_scene->get_system<PhysicsSystem>())
    {
        if (m_physics_system)
            m_collider_id = m_physics_system->register_mobile_collider(*this);

        set_tick_phases(TickPhase::Update);
    }

    ColliderComponent::~ColliderComponent()
    {
        if (m_physics_system)
            m_physics_system->unregister_mobile_collider(m_collider_id);
    }

    void ColliderComponent::on_displaced(const v3& displacement, const v3& normal)
    {
        owner()->transform.position += displacement;
        on_collision.broadcast({ displacement, normal });
    }

    void ColliderComponent::set_size(const v3& size)
    {
        m_size = size;

        if (m_physics_system)
            m_physics_system->update_mobile_collider(m_collider_id);
    }

    void ColliderComponent::set_offset(const v3& offset)
    {
        m_offset = offset;

        if (m_physics_system)
            m_physics_system->update_mobile_collider(m_collider_id);
    }

    physics::Collider_AABB ColliderComponent::calc_collider() const
    {
        return physics::Collider_AABB {
            .position = owner()->transform.position + m_offset,
            .size = m_size
        };
    }

    void ColliderComponent::update(f32 delta_time)
    {
        if (m_previous_location != owner()->transform.position)
        {
            if (m_physics_system)
                m_physics_system->update_mobile_collider(m_collider_id);

            m_previous_location = owner()->transform.position;
        }
    }
}
