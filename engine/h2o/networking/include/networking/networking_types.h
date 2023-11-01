#pragma once

#include "core/types.h"

#include <vector>

namespace h2o
{
    using MsgID = u32;
    using PeerID = u32;

    template<class T>
    using MsgReceivedEventCallback = std::function<void(PeerID, const T&)>;

    enum class ConnectionState
    {
        Disconnected,
        Connecting,
        Connected
    };

    struct ReceivedMessageEvent
    {
        PeerID client_id{};
        const std::vector<u8>& msg;
    };
}