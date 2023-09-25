#pragma once

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/brief_syntax.h>
#include <bitsery/common.h>
#include <bitsery/ext/entropy.h>
#include <bitsery/ext/pointer.h>
#include <bitsery/ext/utils/pointer_utils.h>
#include <bitsery/traits/vector.h>
#include <vector>

namespace h2o::net_utils
{
    using Buffer = std::vector<u8>;
    using Reader = bitsery::InputBufferAdapter<Buffer>;
    using Writer = bitsery::OutputBufferAdapter<Buffer>;

    template<class MsgType>
    bool serialize(const MsgType& msg, Buffer& buffer)
    {
        bitsery::ext::PointerLinkingContext ctx{};
        size_t written_size = bitsery::quickSerialization(ctx, Writer { buffer }, msg);

        assert(ctx.isValid());
        return written_size > 0;
    }

    template<class MsgType>
    bool deserialize(const Buffer& buffer, MsgType& out_result)
    {
        bitsery::ext::PointerLinkingContext ctx{};
        const auto [error, has_error] = bitsery::quickDeserialization(
            ctx, Reader { buffer.begin(), buffer.size() }, out_result);

        assert(ctx.isValid());
        return !has_error;
    }
}