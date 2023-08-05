#pragma once

#include "core/types.h"

#include <functional>
#include <optional>
#include <vector>

class Chunk;

namespace h2o
{
    struct ChunkDistance
    {
        Chunk* chunk { nullptr };
        f32 distance { 0.0f };

        [[nodiscard]] bool operator<(const ChunkDistance& other) const
        { return distance < other.distance; }

    };

    using ChunkDistanceQueue = std::vector<ChunkDistance>;

    namespace voxel
    {
        // Insert chunk based on its distance value
        void chunk_distance_queue_insert(ChunkDistanceQueue& queue, const ChunkDistance& chunk_distance);

        // Pop chunk at closest distance where the predicate is evaluated to true
        std::optional<ChunkDistance> chunk_distance_queue_pop(ChunkDistanceQueue& queue, std::function<bool(const ChunkDistance&)> predicate);
    }
}