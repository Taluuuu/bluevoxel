#include "scene/actors/fps_character_actor.h"

#include "scene/components/camera_component.h"
#include "scene/components/transform_component.h"

namespace h2o
{
    FpsCharacterActor::FpsCharacterActor(const ActorInitializer& actor_initializer)
        : Actor(actor_initializer)
    {
        add_component<TransformComponent>();
        add_component<CameraComponent>();

        //m_input_module = m_engine.
    }

    void FpsCharacterActor::tick(f32 delta_time)
    {

    }
}