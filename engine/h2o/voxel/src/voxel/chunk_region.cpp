#include "voxel/chunk_region.h"

#include "core/engine.h"
#include "voxel/voxel_pack.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    ChunkRegion::ChunkRegion(v2i position)
        : m_position(position)
    {
    }

    void ChunkRegion::register_structures(const std::vector<VoxelStructureInstance>& structures)
    {
        for (const auto& structure : structures)
            m_structures.push_back(structure);
    }
}
