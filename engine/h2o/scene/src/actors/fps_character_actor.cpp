#include "scene/actors/fps_character_actor.h"

#include "scene/components/camera_component.h"

namespace h2o
{
    FpsCharacterActor::FpsCharacterActor(const ActorInitializer& actor_initializer)
        : Actor(actor_initializer)
        , m_camera_component(add_component<CameraComponent>())
    {
    }

    void FpsCharacterActor::tick(f32 delta_time)
    {

    }
}