#include "scene/actor.h"

#include "scene/scene.h"
#include "scene/component.h"

namespace h2o
{
    Actor::Actor(const ActorInitializer& actor_initializer)
        : m_name(actor_initializer.actor_name)
        , m_scene(&actor_initializer.scene)
    {

    }
}