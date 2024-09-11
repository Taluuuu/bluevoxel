#pragma once

#include "core/types.h"
#include "rendering/vertex_array.h"
#include "voxel/chunk_view.h"

#include <vector>

namespace h2o
{
    class RenderingModule;
    class VoxelModule;

    namespace gfx
    {
        class IRenderer;
        class Buffer;
    }

    struct ChunkMeshRenderData
    {
        gfx::VertexArray vertex_array;
        u32 vertex_count = 0;
        v3i chunk_pos{};
    };

    class ChunkMesh
    {
    public:

        ChunkMesh(
            const ChunkView<v3u{3}>& chunk_view,
            const VoxelModule& voxel_module);

        [[nodiscard]] const v3i& chunk_pos() const { return m_chunk_pos; }
        [[nodiscard]] const std::vector<u32>& vertices() const { return m_vertices; }
        [[nodiscard]] u32 vertex_count() const { return m_vertices.size() / 3; }

    private:

        void build_mesh(
            const ChunkView<v3u{3}>& chunk_view,
            const VoxelModule& voxel_rendering_module);

    private:

        std::vector<u32> m_vertices{};

        v3i m_chunk_pos { 0, 0, 0 };

    };
}
