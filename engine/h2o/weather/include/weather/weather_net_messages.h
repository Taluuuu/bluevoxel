#pragma once

#include "networking/message_ids.h"

namespace h2o::net_msg
{
    struct TimeChanged
    {
        i64 new_time = 0;

        template<typename S>
        void serialize(S& s) { s(new_time); }

        static constexpr MsgID message_id = msg_ids::time_changed;
    };
}
