#include "scene/component.h"

#include "scene/actor.h"
#include "scene/scene.h"

namespace h2o
{
    Component::Component(const ComponentInitializer& component_initializer)
        : Tickable(&component_initializer.owner)
        , owner(component_initializer.owner)
        , scene(component_initializer.owner.scene)
    {}
}