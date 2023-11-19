#pragma once

#include "core/handle_types.h"
#include "core/types.h"

#include <glm/gtx/hash.hpp>
#include <map>
#include <mutex>
#include <optional>
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

        [[nodiscard]] size_t num_chunks_in_queue() const { return m_pending_chunks.size(); }

        void enqueue(const v3i& chunk_pos);
        [[nodiscard]] std::optional<v3i> dequeue_first(const std::function<bool(const v3i&)>& condition);

    private:

        std::multimap<f32, v3i> m_chunks_to_mesh_by_distance{};
        std::unordered_set<v3i> m_pending_chunks{};

        WeakHandle<Actor> m_player_actor{};

        std::mutex m_mutex{};

    };
}