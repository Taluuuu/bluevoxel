#pragma once

#include "core/types.h"

#include <vector>

namespace h2o
{
    using MsgID = u32;
    using ClientID = u32;

    template<class T>
    using MsgReceivedEventCallback = std::function<void(ClientID, const T&)>;

    enum class ConnectionState
    {
        Disconnected,
        Connecting,
        Connected
    };

    struct ReceivedMessageEvent
    {
        ClientID client_id{};
        const std::vector<u8>& msg;
    };
}