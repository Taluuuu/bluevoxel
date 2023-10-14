#pragma once

#include "actor_initializer.h"
#include "networking/message_ids.h"
#include "transform.h"

namespace h2o::net_msg
{
    struct PlayerJoin
    {
        ActorID actor_id = 0;
        Transform transform{};

        template<typename S>
        void serialize(S& s)
        { s(actor_id); }

        static constexpr MsgID message_id = msg_ids::player_join;
    };

    struct TransformUpdate
    {
        ActorID actor_id = 0;
        Transform transform{};

        template<typename S>
        void serialize(S& s)
        { s(actor_id, transform); }

        static constexpr MsgID message_id = msg_ids::transform_update;
    };
}