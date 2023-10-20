#include "voxel_client/chunk_mesh_pool.h"

namespace h2o
{
    void ChunkMeshPool::fetch_or_create_chunk_mesh(const v3i& chunk_pos, const std::function<void(ChunkMesh&)>& function)
    {
        std::lock_guard lock { m_mutex };

        ChunkMeshData* mesh_data = find_mesh(chunk_pos);
        if (!mesh_data)
            mesh_data = &create_mesh(chunk_pos);

        assert(mesh_data);

        function(mesh_data->chunk_mesh);
    }

    void ChunkMeshPool::for_each_chunk_mesh(const std::function<void(const ChunkMesh&)>& function) const
    {
        std::lock_guard lock { m_mutex };

        for (const auto& chunk_mesh : m_chunk_mesh_pool)
            function(chunk_mesh.chunk_mesh);
    }

    ChunkMeshData& ChunkMeshPool::create_mesh(const v3i& chunk_pos)
    {
        assert(find_mesh(chunk_pos) == nullptr);

        auto [mesh_data, mesh_id] = reserve_chunk_mesh();
        assign_chunk_mesh(chunk_pos, mesh_id);

        return mesh_data;
    }

    std::pair<ChunkMeshData&, ChunkMeshID> ChunkMeshPool::reserve_chunk_mesh()
    {
        i32 index = 0;
        for (; index < m_chunk_mesh_pool.size(); index++)
        {
            auto& chunk_mesh_data = m_chunk_mesh_pool[index];
            if (chunk_mesh_data.is_available)
            {
                chunk_mesh_data.is_available = false;
                return { chunk_mesh_data, index };
            }
        }

        auto& chunk_mesh_data = m_chunk_mesh_pool.emplace_back();
        chunk_mesh_data.is_available = false;

        return { chunk_mesh_data, index };
    }

    void ChunkMeshPool::assign_chunk_mesh(const v3i& chunk_pos, ChunkMeshID mesh_id)
    {
        m_chunk_mesh_indices[chunk_pos] = mesh_id;
    }

    ChunkMeshData* ChunkMeshPool::find_mesh(const v3i& chunk_pos)
    {
        if (auto mesh_id = find_mesh_id(chunk_pos))
        {
            assert(*mesh_id < m_chunk_mesh_pool.size());
            return &m_chunk_mesh_pool[*mesh_id];
        }

        return nullptr;
    }

    std::optional<ChunkMeshID> ChunkMeshPool::find_mesh_id(const v3i& chunk_pos) const
    {
        if (auto it = m_chunk_mesh_indices.find(chunk_pos); it != m_chunk_mesh_indices.end())
            return it->second;

        return std::nullopt;
    }
}