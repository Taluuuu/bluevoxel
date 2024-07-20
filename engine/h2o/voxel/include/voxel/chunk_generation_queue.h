#pragma once

#include "core/distance_queue.h"
#include "core/handle_types.h"
#include "core/types.h"

#include <functional>

namespace h2o
{
    // class ChunkColumn;

    // using ChunkFetchCallback = std::function<void(const WeakHandle<ChunkColumn>&)>;

    // struct ChunkLoadRequest
    // {
    //     v2i chunk_pos;
    //     ChunkFetchCallback fetch_callback;
    //
    //     bool operator==(const auto& other) const
    //     { return chunk_pos == other.chunk_pos; }
    // };

    class ChunkGenerationQueue
    {
    public:

        ChunkGenerationQueue();

//        DistanceQueue<ChunkLoadRequest>& get_priority_generation_queue();

    private:

        // std::vector< DistanceQueue<ChunkLoadRequest> > m_chunk_gen_requests;

    };
}