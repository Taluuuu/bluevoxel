#include "voxel/chunk_lighting.h"

#include "core/engine.h"
#include "voxel/chunk.h"

namespace h2o
{
    ChunkLighting::ChunkLighting()
    {
        m_light_levels.resize(voxel_constants::chunk_volume, {});
    }

    void ChunkLighting::reset()
    {
        std::ranges::fill(m_light_levels, ChunkLightLevel{});
    }
}
