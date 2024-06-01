#include "voxel_rendering/chunk_meshing_queue.h"

#include "scene/actor.h"
#include "voxel/voxel_utils.h"

#include <glm/gtx/norm.hpp>

namespace h2o
{
    void ChunkMeshingQueue::set_player_actor(const WeakHandle<Actor>& player_actor)
    {
        std::lock_guard lock { m_mutex };
        m_player_actor = player_actor;

        // TODO: Redo queue ?
    }

    void ChunkMeshingQueue::enqueue(const v3i& chunk_pos)
    {
        std::lock_guard lock { m_mutex };

        if (m_pending_chunks.contains(chunk_pos))
            return;

        const v3 chunk_world_pos = voxel_utils::chunk_to_world_pos(chunk_pos);
        const v3 player_pos = m_player_actor ? m_player_actor->transform.position : v3{};
        const f32 sqr_distance = glm::distance2(chunk_world_pos, player_pos);

        m_chunks_to_mesh_by_distance.insert({ sqr_distance, chunk_pos });
        m_pending_chunks.insert(chunk_pos);
    }

    std::optional<v3i> ChunkMeshingQueue::dequeue_first(const std::function<bool(const v3i&)>& condition)
    {
        if (m_chunks_to_mesh_by_distance.empty())
            return std::nullopt;

        for (auto it = m_chunks_to_mesh_by_distance.begin();
            it != m_chunks_to_mesh_by_distance.end(); ++it)
        {
            const v3i chunk_pos = it->second;
            if (condition(chunk_pos))
            {
                m_chunks_to_mesh_by_distance.erase(it);
                m_pending_chunks.erase(chunk_pos);
                return chunk_pos;
            }
        }

        return std::nullopt;
    }
}