#include "scene/component.h"

#include "scene/actor.h"
#include "scene/scene.h"

namespace h2o
{
    Component::Component(const ComponentInitializer& component_initializer)
        : m_owner(component_initializer.owner.observer_from_this())
        , m_scene(&component_initializer.owner.scene())
    {
        assert(m_scene);
    }
}