#include "scene/actors/fps_character_actor.h"

#include "scene/components/camera_component.h"

namespace h2o
{
    FpsCharacterActor::FpsCharacterActor(const ActorInitializer& actor_initializer)
        : Actor(actor_initializer)
    {
        add_component<CameraComponent>();
    }
}