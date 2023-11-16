#include "voxel_rendering/chunk_meshing_queue.h"

namespace h2o
{
    void ChunkMeshingQueue::enqueue(const v3i& chunk_pos, f32 distance)
    {
        std::lock_guard lock { m_mutex };

        if (auto [_, ok] = m_chunks_in_queue.insert(chunk_pos); ok)
            m_chunk_to_mesh_queue.emplace(chunk_pos, distance);
    }

    std::optional<v3i> ChunkMeshingQueue::dequeue()
    {
        std::lock_guard lock { m_mutex };

        if (m_chunk_to_mesh_queue.empty())
            return std::nullopt;

        const auto [chunk_pos, _] = m_chunk_to_mesh_queue.top();
        m_chunk_to_mesh_queue.pop();

        return chunk_pos;
    }
}