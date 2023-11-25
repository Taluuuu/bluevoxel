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
        // TODO: Remove some of these
        class IVertexArray;
        class IBuffer;
        class IRenderer;

        class Buffer;
    }

    class ChunkMesh
    {
    public:

        ChunkMesh(
            const ChunkRegion& chunk_region,
            const VoxelRenderingModule& voxel_rendering_module);

        void update_buffer(gfx::Buffer& vertex_buffer);

        [[nodiscard]] const v3i& chunk_pos()    const { return m_chunk_pos;         }
        [[nodiscard]]       i32  vertex_count() const { return m_vertex_count;      }
        [[nodiscard]]       bool is_empty()     const { return m_vertex_count == 0; }

    private:

        void build_mesh(
            const ChunkRegion& chunk_region,
            const VoxelRenderingModule& voxel_rendering_module);

    private:

        std::vector<u32> m_pending_vertices{};

        v3i m_chunk_pos { 0, 0, 0 };
        i32 m_vertex_count = 0;

    };
}