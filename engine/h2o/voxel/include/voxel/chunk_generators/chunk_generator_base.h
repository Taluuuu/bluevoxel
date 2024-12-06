#pragma once

#include "voxel/chunk_manager.h"
#include "voxel/structures/voxel_structure_manager.h"

#include <vector>

namespace h2o
{
    class ChunkGenerator_Base
    {
    public:

        virtual ~ChunkGenerator_Base() = default;

        // Generate blocks for a whole chunk region
        virtual void gen_blocks(const ChunkManager::View<Chunk>& region_view) const = 0;

        // Generate the structure instances for a whole chunk region.
        // These structures can span multiple regions.
        // This assumes the blocks are already generated.
        [[nodiscard]] virtual std::vector<VoxelStructureInstance> gen_structures(const ChunkManager::View<const Chunk>& region_view) const = 0;

    };
}
