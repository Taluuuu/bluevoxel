#pragma once

#include "core/types.h"

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
        class IVertexArray;
        class IBuffer;
        class IRenderer;
    }

    class ChunkMesh
    {
    public:

        ChunkMesh() = default;
        ChunkMesh(ChunkMesh&& other);

        void init(
            const VoxelRenderingModule& voxel_rendering_module,
            const RenderingModule& rendering_module);

        void generate_vertices(const ChunkRegion& chunk_region);
        void update_mesh();

        [[nodiscard]] const v3i& chunk_pos()    const { return m_chunk_pos;         }
        [[nodiscard]]       i32  vertex_count() const { return m_vertex_count;      }
        [[nodiscard]]       bool is_empty()     const { return m_vertex_count == 0; }
        [[nodiscard]] const gfx::IVertexArray& vertex_array() const;

    private:

        std::shared_ptr<gfx::IVertexArray> m_vertex_array = nullptr;
        std::shared_ptr<gfx::IBuffer> m_buffer = nullptr;

        const VoxelRenderingModule* m_voxel_rendering_module = nullptr;
        const RenderingModule* m_rendering_module = nullptr;

        std::vector<u32> m_pending_vertices{};
        mutable std::mutex m_vertices_mutex;

        v3i m_chunk_pos { 0, 0, 0 };
        i32 m_vertex_count = 0;

    };
}