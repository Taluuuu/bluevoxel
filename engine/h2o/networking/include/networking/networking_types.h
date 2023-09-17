#pragma once

#include "core/types.h"

#include <vector>

namespace h2o
{
    using MsgID = u32;
    using ClientID = u32;

    enum class ConnectionState
    {
        Disconnected,
        Connecting,
        Connected
    };

    struct ReceivedMessageEvent
    {
        const std::vector<u8>& msg;
    };
}