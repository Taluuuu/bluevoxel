#pragma once

#include "message_ids.h"

#include <bitsery/brief_syntax/string.h>

namespace h2o
{
    struct TestMessage
    {
        std::string text{};
        u32 num = 0;

        template<typename S>
        void serialize(S& s)
        {
            s(text, num);
        }

        static constexpr MsgID message_id = msg_ids::test;
    };
}