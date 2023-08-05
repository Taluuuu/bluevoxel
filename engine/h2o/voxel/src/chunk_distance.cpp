#include "voxel/chunk_distance.h"

#include "voxel/chunk.h"

namespace h2o::voxel
{
    bool chunk_distance_queue_contains(const ChunkDistanceQueue& queue, const Chunk& chunk)
    {
        auto found_chunk_mesh = std::find_if(queue.begin(), queue.end(),
            [&chunk](const ChunkDistance& item) -> bool
            { return item.chunk == &chunk; }
        );

        return found_chunk_mesh != queue.end();
    }

    void chunk_distance_queue_insert(ChunkDistanceQueue& queue, const ChunkDistance& chunk_distance)
    {
        for (size_t i = 0; i < queue.size(); i++)
        {
            if (chunk_distance.distance < queue[i].distance)
            {
                queue.insert(queue.cbegin() + i, chunk_distance);
                return;
            }
        }

        queue.push_back(chunk_distance);
    }

    // Pop chunk at closest distance where the predicate is evaluated to true
    std::optional<ChunkDistance> chunk_distance_queue_pop(ChunkDistanceQueue& queue, std::function<bool(const ChunkDistance&)> predicate)
    {
        for (size_t i = 0; i < queue.size(); i++)
        {
            ChunkDistance chunk_distance = queue[i];
            if (predicate(chunk_distance))
            {
                queue.erase(queue.cbegin() + i);
                return chunk_distance;
            }
        }

        return std::nullopt;
    }
}