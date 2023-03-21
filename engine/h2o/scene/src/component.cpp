#include "scene/component.h"

namespace h2o
{
    Component::Component(const ComponentInitializer& component_initializer)
        : m_owner(component_initializer.owner)
    {

    }
}