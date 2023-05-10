#pragma once

#include <memory>

namespace h2o
{
    class Chunk;

    namespace gfx
    {
        class IVertexArray;
        class IBuffer;
        class IRenderer;
    }

    class ChunkMesh
    {
    public:

        explicit ChunkMesh(gfx::IRenderer& renderer);

        void update(const Chunk& chunk);

    private:

        std::shared_ptr<gfx::IVertexArray> m_vertex_array = nullptr;
        std::shared_ptr<gfx::IBuffer> m_buffer = nullptr;

    };
}