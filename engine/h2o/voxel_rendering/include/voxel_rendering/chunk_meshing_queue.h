#pragma once

#include "core/handle_types.h"
#include "core/types.h"

#include <glm/gtx/hash.hpp>
#include <mutex>
#include <optional>
#include <queue>
#include <unordered_set>

namespace h2o
{
    class Actor;

    // Thread-safe chunk meshing priority queue with no duplicates.
    class ChunkMeshingQueue
    {
    public:

        ChunkMeshingQueue() = default;

        /**
         * Player actor is used to prioritize near chunks in the dequeue order
         *
         * @param player_actor The player actor
         */
        void set_player_actor(const WeakHandle<Actor>& player_actor);

        [[nodiscard]] size_t num_chunks_in_queue() const { return m_chunks_in_queue.size(); }

        void enqueue(const v3i& chunk_pos);
        [[nodiscard]] std::optional<v3i> dequeue_if(const std::function<bool(const v3i&)>& condition);

    private:

        struct ChunkPosDistancePair
        {
            v3i chunk_pos{};
            f32 sqr_distance = 0.0f;

            [[nodiscard]] bool operator<(const ChunkPosDistancePair& other) const
            { return sqr_distance > other.sqr_distance; }
        };

        std::priority_queue<ChunkPosDistancePair> m_chunk_to_mesh_queue{};
        std::unordered_set<v3i> m_chunks_in_queue{};

        WeakHandle<Actor> m_player_actor{};

        std::mutex m_mutex{};

    };
}