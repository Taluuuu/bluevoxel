#pragma once

#include "networking/message_ids.h"
#include "voxel/chunk_column.h"

#include <bitsery/brief_syntax.h>
#include <bitsery/brief_syntax/vector.h>

namespace h2o
{
    struct ChunkColPos
    {
        i32 x{}, z{};

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
        ChunkColumn fetched_chunk;

        template<typename S>
        void serialize(S& s)
        { s(fetched_chunk); }
    };
}