#include "scene/actor.h"

#include "scene/scene.h"
#include "scene/component.h"
#include "scene/scene_networking_system.h"

namespace h2o
{
    Actor::Actor(const ActorInitializer& actor_initializer)
        : Tickable(&actor_initializer.scene)
        , m_actor_id(actor_initializer.actor_id)
        , m_scene(&actor_initializer.scene)
    {}

    void Actor::tag_actor(ActorTag tag)
    {
        assert(m_scene);
        m_scene->tag_actor(observer_from_this(), tag);
    }

    void Actor::set_replicate_transform(bool replicate)
    {
        assert(m_scene);
        
        if (auto scene_networking = m_scene->get_system<SceneNetworkingSystem>())
            scene_networking->replicate_actor_transform(observer_from_this());
    }
}