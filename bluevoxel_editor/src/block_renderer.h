#pragma once

#include "voxel/chunk_view.h"

#include <memory>

namespace h2o
{
    class Chunk;
    class ChunkMeshPool;
    class VoxelBounds;
    class VoxelModule;

    namespace gfx
    {
        class IRenderer;
    }
}

namespace bluevoxel
{
    class BlockRenderer
    {
    public:

        BlockRenderer();

        void set_block(h2o::Block block);
        void refresh();

        void render(h2o::gfx::IRenderer& renderer);

    public:

        v3 light_dir { 0.18f, -1.0f, 0.492f };
        v3 light_color { 1.0f, 1.0f, 1.0f };
        f32 ambient_strength = 0.714f;

    private:

        std::shared_ptr<h2o::Chunk> m_chunk = nullptr;
        std::shared_ptr<h2o::ChunkView<v3u{3}>> m_chunk_view = nullptr;
        std::shared_ptr<h2o::VoxelBounds> m_voxel_bounds = nullptr;
        std::shared_ptr<h2o::ChunkMeshPool> m_chunk_mesh_pool = nullptr;

        h2o::VoxelModule* const m_voxel_rendering_module = nullptr;

    };
}