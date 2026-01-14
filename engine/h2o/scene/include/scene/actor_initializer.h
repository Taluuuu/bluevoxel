#pragma once

#include "core/types.h"

#include <string_view>

namespace h2o
{
    class Scene;

    using ActorID = u32;

    enum class ActorTag
    {
        None,
        LocalPlayer
    };

    struct ActorInitializer
    {
        ActorID actor_id = 0;
        Scene& scene;
        u32 owning_peer_id = 0;
    };
}