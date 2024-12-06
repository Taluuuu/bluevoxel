#pragma once

#include "core/types.h"
#include "rendering/vertex_array.h"
#include "voxel/chunk_manager.h"

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
        gfx::VertexArray vertex_array{};
        u32 vertex_count = 0;
        v3i chunk_pos{};

        explicit ChunkMeshRenderData(const v3i& chunk_pos)
            : chunk_pos(chunk_pos)
        {}

        ChunkMeshRenderData(ChunkMeshRenderData&& other) noexcept
            : vertex_array(std::move(other.vertex_array))
            , vertex_count(other.vertex_count)
            , chunk_pos(other.chunk_pos)
        {
            other.chunk_pos = {};
            other.vertex_count = 0;
        }

        ChunkMeshRenderData& operator=(ChunkMeshRenderData&& other) noexcept
        {
            if (this != &other)
            {
                vertex_array = std::move(other.vertex_array);
                vertex_count = other.vertex_count;
                chunk_pos = other.chunk_pos;

                other.vertex_count = 0;
                other.chunk_pos = {};
            }

            return *this;
        }
    };

    class ChunkMesh
    {
    public:

        ChunkMesh(
            const ChunkManager::View<const Chunk>& chunk_view,
            const ChunkLighting& chunk_lighting,
            const VoxelModule& voxel_module);

        [[nodiscard]] const v3i& chunk_pos() const { return m_chunk_pos; }
        [[nodiscard]] const std::vector<u32>& vertices() const { return m_vertices; }
        [[nodiscard]] u32 vertex_count() const { return m_vertices.size() / 3; }

    private:

        void build_mesh(
            const ChunkManager::View<const Chunk>& chunk_view,
            const ChunkLighting& chunk_lighting,
            const VoxelModule& voxel_module);

    private:

        std::vector<u32> m_vertices{};

        v3i m_chunk_pos { 0, 0, 0 };

    };
}
