#pragma once

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/brief_syntax.h>
#include <bitsery/traits/vector.h>

#include <vector>

namespace h2o::net_utils
{
    template<class MsgType>
    bool deserialize(const std::vector<u8>& buffer, MsgType& out_result)
    {
        using Buffer = std::vector<u8>;
        using InputAdapter = bitsery::InputBufferAdapter<Buffer>;

        const auto [error, has_error] = bitsery::quickDeserialization<InputAdapter>(
            { buffer.begin(), buffer.size() }, out_result);

        return !has_error;
    }
}