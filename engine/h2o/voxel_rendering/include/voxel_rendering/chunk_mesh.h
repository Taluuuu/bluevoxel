#pragma once

#include "core/types.h"
#include "rendering/vertex_array.h"

#include <memory>
#include <mutex>
#include <vector>

namespace h2o
{
    class ChunkRegion;
    class RenderingModule;
    class VoxelRenderingModule;

    namespace gfx
    {
        class IRenderer;
        class Buffer;
    }

    class ChunkMesh
    {
    public:

        ChunkMesh(
            const ChunkRegion& chunk_region,
            const VoxelRenderingModule& voxel_rendering_module);

        [[nodiscard]] const v3i& chunk_pos() const { return m_chunk_pos; }
        [[nodiscard]] const std::vector<u32>& vertices() const { return m_vertices; }
        [[nodiscard]] u32 vertex_count() const { return m_vertices.size() / 3; }

    private:

        void build_mesh(
            const ChunkRegion& chunk_region,
            const VoxelRenderingModule& voxel_rendering_module);

    private:

        std::vector<u32> m_vertices{};

        v3i m_chunk_pos { 0, 0, 0 };

    };
}