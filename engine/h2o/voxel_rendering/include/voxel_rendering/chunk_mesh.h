#pragma once

#include "core/types.h"

#include <array>
#include <memory>

namespace h2o
{
    class Chunk;
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
        ChunkMesh(ChunkMesh&& other) noexcept;
        ChunkMesh& operator=(ChunkMesh&& other) noexcept;

        void init(gfx::IRenderer& renderer, const v3i& chunk_pos);
        void update(
            const VoxelRenderingModule& chunk_rendering_module,
            const Chunk& chunk,
            const std::array<Chunk*, 6>& adjacent_chunks);

        [[nodiscard]] const v3i& chunk_pos()    const { return m_chunk_pos;               }
        [[nodiscard]]       i32  vertex_count() const { return m_vertex_count;            }
        [[nodiscard]]       bool is_ready()     const { return m_vertex_array != nullptr; }
        [[nodiscard]] const gfx::IVertexArray& vertex_array() const;

    private:

        std::shared_ptr<gfx::IVertexArray> m_vertex_array = nullptr;
        std::shared_ptr<gfx::IBuffer> m_buffer = nullptr;

        v3i m_chunk_pos { 0, 0, 0 };
        i32 m_vertex_count { 0 };

    };
}