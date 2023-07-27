#include "voxel_rendering/chunk_rendering_region.h"

#include "core/log.h"
#include "voxel/chunk.h"
#include "voxel/chunk_system.h"

namespace h2o
{
    ChunkRenderingRegion::ChunkRenderingRegion(
        gfx::IRenderer& renderer,
        const VoxelRenderingModule& voxel_rendering_module,
        const WeakHandle<ChunkSystem>& chunk_system)
        : ChunkRegion(chunk_system)
        , m_renderer(&renderer)
        , m_voxel_rendering_module(&voxel_rendering_module)
    {
        chunk_system->on_chunk_updated.add_listener(m_on_chunk_updated_handle,
            [&](const ChunkEvent& event)
            {
                const v3i& chunk_pos = event.chunk.chunk_pos();
                if (!in_region_bounds(chunk_pos))
                    return;

                const v3i local_pos = to_local_chunk_pos(chunk_pos);
                const size_t index = to_index({ local_pos.x, local_pos.z });
                const auto& mesh_column = m_chunk_mesh_columns[index];
                if (!mesh_column)
                    return;

                auto& chunk_mesh = (*mesh_column)[chunk_pos.y];

                chunk_mesh.update(voxel_rendering_module, event.chunk);
            });
    }

    void ChunkRenderingRegion::for_each_chunk_mesh(const std::function<void(ChunkMesh&)>& fun) const
    {
        for (auto& chunk_mesh_column : m_chunk_mesh_columns)
        {
            if (chunk_mesh_column)
            {
                for (auto& chunk_mesh : *chunk_mesh_column)
                    fun(chunk_mesh);
            }
        }
    }

    void ChunkRenderingRegion::on_indices_changed(const std::vector<i32>& new_to_old_indices)
    {
        assert(m_voxel_rendering_module);

        std::vector<ChunkMeshColumnPtr> new_chunk_meshes(new_to_old_indices.size());

        for (i32 i = 0; i < size(); i++)
        for (i32 k = 0; k < size(); k++)
        {
            const i32 new_idx = i * i32(size()) + k;
            const i32 old_idx = new_to_old_indices[new_idx];

            if (old_idx != -1)
            {
                new_chunk_meshes[new_idx] = std::move(m_chunk_mesh_columns[old_idx]);
            }
        }

        m_chunk_mesh_columns = std::move(new_chunk_meshes);
    }

    void ChunkRenderingRegion::on_chunk_fetched(const ChunkColumnPtr& chunk_col, v2i chunk_col_pos)
    {
        // Make new chunk mesh column
        auto new_column = std::make_unique<ChunkMeshColumn>();
        for (i32 j = 0; j < new_column->size(); j++)
        {
            const auto& chunk = (*chunk_col)[j];
            auto& chunk_mesh = (*new_column)[j];

            chunk_mesh.init(*m_renderer, chunk.chunk_pos());
            chunk_mesh.update(*m_voxel_rendering_module, chunk);
        }

        // TODO: Create chunk column class
        assert(in_region_bounds(chunk_col_pos));
        const v2i rel_chunk_col_pos = chunk_col_pos - corner_pos();
        const size_t idx = rel_chunk_col_pos.x * size() + rel_chunk_col_pos.y;

        m_chunk_mesh_columns[idx] = std::move(new_column);
    }
}