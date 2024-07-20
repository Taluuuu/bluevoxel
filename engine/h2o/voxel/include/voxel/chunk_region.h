#pragma once

#include <vector>

#include "structures/voxel_structure_manager.h"

namespace h2o
{
    class Chunk;
    class VoxelStructure;

    class ChunkRegion
    {
    public:

        explicit ChunkRegion(v3i position);

        // Places the bit of contained structures that fits in this chunk
        void place_structure(Chunk& chunk) const;

    private:

        std::vector<VoxelStructureInstance> m_structures{};
        v3i m_position{};

    };
}
