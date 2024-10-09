#include "physics/scene/collider_component.h"

#include "physics/scene/physics_system.h"
#include "scene/actor.h"
#include "scene/scene.h"

namespace h2o
{
    ColliderComponent::ColliderComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
    {
        const auto physics_system = m_scene->get_system<PhysicsSystem>();
        physics_system->register_mobile_collider(*this);

        set_tick_phases(TickPhase::Update);
    }

    ColliderComponent::~ColliderComponent()
    {
        const auto physics_system = m_scene->get_system<PhysicsSystem>();
        physics_system->unregister_mobile_collider(*this);
    }

    void ColliderComponent::update(f32 delta_time)
    {
        owner()->transform.position = collider.position;
    }
}
