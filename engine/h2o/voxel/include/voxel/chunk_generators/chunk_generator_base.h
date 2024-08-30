#pragma once

#include "core/types.h"
#include "voxel/chunk_region.h"
#include "voxel/structures/voxel_structure_manager.h"

#include <vector>

namespace h2o
{
    class ChunkGenerator_Base
    {
    public:

        virtual ~ChunkGenerator_Base() = default;

        // Generate blocks for a whole chunk region
        virtual void gen_blocks(ChunkRegionView& region_view) const = 0;

        // Generate the structure instances for a whole chunk region.
        // These structures can span multiple regions.
        // This assumes the blocks are already generated.
        [[nodiscard]] virtual std::vector<VoxelStructureInstance> gen_structures(const ChunkRegionView& region_view) const = 0;

    };
}
