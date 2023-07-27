#pragma once

#include "chunk_mesh.h"
#include "core/events.h"
#include "voxel/chunk_region.h"

#include <array>
#include <memory>

namespace h2o
{
    class ChunkRenderingRegion : public ChunkRegion
    {
    public:

        ChunkRenderingRegion(
            gfx::IRenderer& renderer,
            const VoxelRenderingModule& voxel_rendering_module,
            const WeakHandle<ChunkSystem>& chunk_system);

        void for_each_chunk_mesh(const std::function<void(ChunkMesh&)>& fun) const;

    protected:

        // ChunkRegion interface
        void on_indices_changed(const std::vector<i32>& new_to_old_indices) override;
        void on_chunk_fetched(const ChunkColumnPtr& chunk_col, v2i local_chunk_pos) override;

    private:

        using ChunkMeshColumn = std::array<ChunkMesh, voxel_constants::vertical_chunk_count>;
        // Chunk mesh columns should be easy to move.
        using ChunkMeshColumnPtr = std::unique_ptr<ChunkMeshColumn>;
        std::vector<ChunkMeshColumnPtr> m_chunk_mesh_columns;

        EventHandle m_on_chunk_updated_handle;

        gfx::IRenderer* const m_renderer = nullptr;
        const VoxelRenderingModule* const m_voxel_rendering_module = nullptr;

    };
}