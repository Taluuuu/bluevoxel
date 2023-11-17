#include "voxel_rendering/chunk_meshing_queue.h"

#include "scene/actor.h"
#include "voxel/voxel_utils.h"

#include <glm/gtx/norm.hpp>

namespace h2o
{
    void ChunkMeshingQueue::set_player_actor(const WeakHandle<Actor>& player_actor)
    {
        m_player_actor = player_actor;

        // TODO: Redo queue ?
    }

    void ChunkMeshingQueue::enqueue(const v3i& chunk_pos)
    {
        std::lock_guard lock { m_mutex };

        if (auto [_, ok] = m_chunks_in_queue.insert(chunk_pos); ok)
        {
            const v3 chunk_world_pos = voxel_utils::chunk_to_world_pos(chunk_pos);
            const v3 player_pos = m_player_actor ? m_player_actor->transform.position : v3{};

            const f32 sqr_distance = glm::distance2(chunk_world_pos, player_pos);
            m_chunk_to_mesh_queue.emplace(chunk_pos, sqr_distance);
        }
    }

    std::optional<v3i> ChunkMeshingQueue::dequeue_if(const std::function<bool(const v3i&)>& condition)
    {
        if (m_chunk_to_mesh_queue.empty())
            return std::nullopt;

        if (!condition(m_chunk_to_mesh_queue.top().chunk_pos))
            return std::nullopt;

        const auto [chunk_pos, _] = m_chunk_to_mesh_queue.top();
        m_chunk_to_mesh_queue.pop();
        m_chunks_in_queue.erase(chunk_pos);

        return chunk_pos;
    }
}