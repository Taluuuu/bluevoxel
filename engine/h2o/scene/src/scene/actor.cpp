#include "scene/actor.h"

#include "scene/scene.h"
#include "scene/component.h"
#include "scene/scene_networking_system.h"

namespace h2o
{
    Actor::Actor(const ActorInitializer& actor_initializer)
        : Tickable(&actor_initializer.scene)
        , scene(actor_initializer.scene)
        , m_actor_id(actor_initializer.actor_id)
        , owning_peer_id(actor_initializer.owning_peer_id)
    {}

    void Actor::start()
    {
        for (const auto& component : m_components)
            component->start();
    }

    void Actor::tag_actor(ActorTag tag)
    {
        scene.tag_actor(observer_from_this(), tag);
    }

    void Actor::set_replicate_transform(bool replicate)
    {
        if (const auto scene_networking = scene.get_system<SceneNetworkingSystem>())
            scene_networking->replicate_actor_transform(observer_from_this());
    }
}