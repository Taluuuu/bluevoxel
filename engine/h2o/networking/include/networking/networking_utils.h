#pragma once

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/brief_syntax.h>
#include <bitsery/traits/vector.h>

#include <vector>

namespace h2o::net_utils
{
    using Buffer = std::vector<u8>;
    using InputAdapter = bitsery::InputBufferAdapter<Buffer>;
    using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;

    template<class MsgType>
    bool serialize(const MsgType& msg, Buffer& buffer)
    {
        return bitsery::quickSerialization<OutputAdapter>(buffer, msg) > 0;
    }

    template<class MsgType>
    bool deserialize(const Buffer& buffer, MsgType& out_result)
    {
        const auto [error, has_error] = bitsery::quickDeserialization<InputAdapter>(
            { buffer.begin(), buffer.size() }, out_result);

        return !has_error;
    }
}