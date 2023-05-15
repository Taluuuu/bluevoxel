#pragma once

#include "core/types.h"

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

        // TODO: Create a factory function returning an std::optional<ChunkMesh>? This could
        //       allow skipping a null check in the rendering loop

        ChunkMesh(gfx::IRenderer& renderer, const v3i& chunk_pos);
        ChunkMesh(ChunkMesh&& other) noexcept;
        ChunkMesh& operator=(ChunkMesh&& other) noexcept;

        void update(const VoxelRenderingModule& chunk_rendering_module, const Chunk& chunk);

        [[nodiscard]] const v3i& chunk_pos() const { return m_chunk_pos; }
        [[nodiscard]] const gfx::IVertexArray& vertex_array() const;

    public:

        std::shared_ptr<gfx::IVertexArray> m_vertex_array = nullptr;
        std::shared_ptr<gfx::IBuffer> m_buffer = nullptr;

        v3i m_chunk_pos;

    };
}