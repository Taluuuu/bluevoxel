#include "voxel/chunk_generators/chunk_generator_flat.h"

#include "voxel/voxel_constants.h"

namespace h2o
{
    void ChunkGenerator_Flat::gen_blocks(ChunkRegionView& region_view) const
    {
        for (i32 x = 0; x < voxel_constants::chunk_region_block_count; x++)
        for (i32 z = 0; z < voxel_constants::chunk_region_block_count; z++)
        {
            i32 y = 0;
            for (const Block& layer : block_layers)
            {
                region_view.set_block_at({x, y, z}, layer, ViewRelativeTo::ViewCorner);
                y++;
            }
        }

        // For testing
        region_view.set_block_at({ 0, block_layers.size(), 0 }, { 2 }, ViewRelativeTo::ViewCorner);
    }

    std::vector<VoxelStructureInstance> ChunkGenerator_Flat::gen_structures(const ChunkRegionView& region_view) const
    {
        return {};
    }
}
