#pragma once

#include "core/types.h"

#include <glm/gtx/hash.hpp>
#include <mutex>
#include <optional>
#include <queue>
#include <unordered_set>

namespace h2o
{
    // Thread-safe chunk meshing priority queue with no duplicates.
    class ChunkMeshingQueue
    {
    public:

        ChunkMeshingQueue() = default;

        void enqueue(const v3i& chunk_pos, f32 distance);
        std::optional<v3i> dequeue();

    private:

        struct ChunkPosDistancePair
        {
            v3i chunk_pos{};
            f32 distance = 0.0f;

            [[nodiscard]] bool operator<(const ChunkPosDistancePair& other) const
            { return distance > other.distance; }
        };

        std::priority_queue<ChunkPosDistancePair> m_chunk_to_mesh_queue{};
        std::unordered_set<v3i> m_chunks_in_queue{};

        std::mutex m_mutex{};

    };
}