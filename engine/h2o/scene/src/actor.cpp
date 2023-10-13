#include "scene/actor.h"

#include "scene/scene.h"
#include "scene/component.h"

namespace h2o
{
    Actor::Actor(const ActorInitializer& actor_initializer)
        : m_actor_id(actor_initializer.actor_id)
        , m_scene(&actor_initializer.scene)
    {

    }

    void Actor::tag_actor(ActorTag tag)
    {
        assert(m_scene);
        m_scene->tag_actor(observer_from_this(), tag);
    }
}