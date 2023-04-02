#include "game_framework/actors/triangle_actor.h"

#include "core/engine.h"
#include "rendering/rendering_module.h"

namespace h2o
{
    TriangleActor::TriangleActor(const ActorInitializer& actor_initializer) : Actor(actor_initializer)
    {
        g_engine->register_tickable(this, TickPhase::Render);



    }

    void TriangleActor::tick(TickPhase phase, f64 delta_time)
    {

    }
}