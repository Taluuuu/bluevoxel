#pragma once

#include "networking/message_ids.h"

namespace h2o
{
    struct WelcomeMsg
    {
        PeerID assigned_id = 0;

        template<typename S>
        void serialize(S& s)
        { s(assigned_id); }

        static constexpr MsgID message_id = msg_ids::welcome;
    };
}
