#include "voxel_rendering/voxel_world_renderer.h"

namespace h2o
{
    VoxelWorldRenderer::VoxelWorldRenderer(Tickable& owner, const ChunkManager& chunk_manager)
        : Tickable(&owner)
        , m_chunk_manager(&chunk_manager)
    {
        set_tick_phases(TickPhase::Render);
    }

    void VoxelWorldRenderer::render()
    {

    }
}
