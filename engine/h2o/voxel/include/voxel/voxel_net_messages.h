#pragma once

#include "networking/message_ids.h"
#include "voxel/chunk_column.h"

#include <bitsery/bitsery.h>
#include <bitsery/brief_syntax.h>
#include <bitsery/brief_syntax/array.h>
#include <bitsery/brief_syntax/memory.h>
#include <bitsery/brief_syntax/vector.h>
#include <bitsery/ext/compact_value.h>
#include <bitsery/ext/entropy.h>
#include <bitsery/ext/pointer.h>
#include <memory>

namespace h2o
{
    struct NetMsg_ChunkFetchRequest
    {
        std::vector<v2i> requested_chunks{};

        template<typename S>
        void serialize(S& s)
        { s(requested_chunks); }

        static constexpr MsgID message_id = msg_ids::chunk_fetch_request;
    };

    struct NetMsg_ChunkFetchResult
    {
        std::vector<CompressedChunk> compressed_chunks{};

        v2i chunk_pos{};

        template<typename S>
        void serialize(S& s)
        { s(compressed_chunks, chunk_pos); }

        static constexpr MsgID message_id = msg_ids::chunk_fetch_result;
    };

    struct NetMsg_BlockPlaceRequest
    {
        Block placed_block{};

        v3i block_pos{};

        template<typename S>
        void serialize(S& s)
        { s(block_pos, placed_block); }

        static constexpr MsgID message_id = msg_ids::block_placed;
    };
}

namespace bitsery
{
    template<typename S>
    void serialize(S& s, v2i& o)
    { s(o.x, o.y); }

    template<typename S>
    void serialize(S& s, v3i& o)
    { s(o.x, o.y, o.z); }
}