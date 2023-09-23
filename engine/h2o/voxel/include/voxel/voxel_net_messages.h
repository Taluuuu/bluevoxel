#pragma once

#include "networking/message_ids.h"
#include "voxel/chunk_column.h"

#include <bitsery/bitsery.h>
#include <bitsery/brief_syntax.h>
#include <bitsery/brief_syntax/array.h>
#include <bitsery/brief_syntax/memory.h>
#include <bitsery/brief_syntax/vector.h>
#include <bitsery/ext/pointer.h>
#include <memory>

namespace h2o
{
    struct ChunkColPos
    {
        i32 x{}, z{};

        ChunkColPos() = default;

        ChunkColPos(i32 x, i32 z)
            : x { x }, z { z } {}

        ChunkColPos(v2i chunk_col_pos)
            : x { chunk_col_pos.x }, z { chunk_col_pos.y } {}

        operator v2i() const
        { return { x, z }; }

        template<typename S>
        void serialize(S& s)
        { s(x, z); }
    };

    struct NetMsg_ChunkFetchRequest
    {
        std::vector<ChunkColPos> requested_chunks{};

        template<typename S>
        void serialize(S& s)
        { s(requested_chunks); }

        static constexpr MsgID message_id = msg_ids::chunk_fetch_request;
    };

    struct NetMsg_ChunkFetchResult
    {
        ChunkColumn* fetched_chunk;

        template<typename S>
        void serialize(S& s)
        { s.ext(fetched_chunk, bitsery::ext::PointerObserver{}); }

        static constexpr MsgID message_id = msg_ids::chunk_fetch_result;
    };
}