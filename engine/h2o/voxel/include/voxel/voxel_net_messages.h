#pragma once

#include "networking/message_ids.h"
#include "voxel/chunk_column.h"

namespace h2o::voxel::net
{
    struct ChunkColPos
    {
        i32 x{}, z{};

        template<typename S>
        void serialize(S& s)
        {
            s(x, z);
        }
    };

    struct ChunkFetchRequest
    {
        std::vector<ChunkColPos> requested_chunks{};

        template<typename S>
        void serialize(S& s)
        {
            s(requested_chunks);
        }

        static constexpr MsgID message_id = msg_ids::chunk_fetch_request;
    };

    struct ChunkFetchResult
    {
        ChunkColumn fetched_chunk;

        template<typename S>
        void serialize(S& s)
        {
            s(fetched_chunk);
        }
    };
}