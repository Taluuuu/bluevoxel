#pragma once

#include "core/types.h"

#include <bitsery/brief_syntax/string.h>
#include <vector>

namespace h2o
{
    using MsgID = u32;

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

    struct TestMessage
    {
        std::string text{};
        u32 num = 0;

        template<typename S>
        void serialize(S& s)
        {
            s(text, num);
        }

        static constexpr MsgID message_id = 312;
    };
}