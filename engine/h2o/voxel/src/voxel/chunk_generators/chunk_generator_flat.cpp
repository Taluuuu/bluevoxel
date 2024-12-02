#include "voxel/chunk_generators/chunk_generator_flat.h"

#include "voxel/chunk_view.h"
#include "voxel/voxel_constants.h"

namespace h2o
{
    void ChunkGenerator_Flat::gen_blocks(ChunkView& region_view) const
    {
        for (i32 x = 0; x < voxel_constants::chunk_region_block_count; x++)
        for (i32 z = 0; z < voxel_constants::chunk_region_block_count; z++)
        {
            i32 y = 0;
            for (const Block& layer : block_layers)
            {
                voxel::set_block_at(region_view, {x, y, z}, layer, EViewRelativeTo::ViewCorner);
                y++;
            }
        }

        // For testing
        // region_view.set_block_at({ 0, block_layers.size(), 0 }, { 2 }, ViewRelativeTo::ViewCorner);
    }

    std::vector<VoxelStructureInstance> ChunkGenerator_Flat::gen_structures(const ChunkView& region_view) const
    {
        std::vector<VoxelStructureInstance> structures{};

        const v3i corner = region_view.corner_cell_pos() * voxel_constants::chunk_size;

        srand(corner.x ^ corner.z);
        for (i32 i = 0; i < voxel_constants::chunk_region_block_count; i++)
        for (i32 j = 0; j < voxel_constants::chunk_region_block_count; j++)
        {
            const f32 random_float = static_cast<f32>(rand()) / static_cast<f32>(std::numeric_limits<i32>::max());
            if (random_float > 0.001f)
                continue;

            // Find ground level. This should probably be an easily accessible function
            i32 ground_level = 0;
            for (; ground_level < voxel_constants::vertical_block_count; ground_level++)
            {
                const auto block = voxel::get_block_at(region_view, { i, ground_level, j }, EViewRelativeTo::ViewCorner);
                if (!block || block == Block::Air)
                    break;
            }

            const v3i structure_pos = corner + v3i{ i, ground_level, j };
            structures.emplace_back(1, structure_pos);
        }

        return structures;
    }
}
