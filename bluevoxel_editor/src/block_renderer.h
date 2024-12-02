#pragma once

#include "voxel/chunk_manager.h"
#include "voxel_rendering/voxel_world_renderer.h"

namespace bluevoxel
{
    class BlockRenderer
    {
    public:

        explicit BlockRenderer(h2o::Tickable& owner);

        void set_block(h2o::Block block);

    private:

        h2o::ChunkManager m_chunk_manager{};
        h2o::VoxelWorldRenderer m_voxel_world_renderer;

    };
}