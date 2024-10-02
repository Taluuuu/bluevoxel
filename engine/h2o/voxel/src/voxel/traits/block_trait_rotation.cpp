#include "voxel/traits/block_trait_rotation.h"

#include "voxel/block_model.h"

namespace h2o
{
    BlockTrait_Rotation::BlockTrait_Rotation(const BlockType& block_type, const u16 offset)
        : BlockTrait(block_type, offset, BlockTrait_Rotation::num_bits())
    {}

    void BlockTrait_Rotation::edit_block_model(const Block block, BlockModel& block_model) const
    {
        const u16 rotation = read_data(block);

        // Garbage, just for testing
        // The values might need baking
        for (auto& side : block_model.occluded_triangles_per_side)
        {
            for (auto& triangle : side)
            {
                for (auto& vertex : triangle)
                    vertex.y /= 2;
            }
        }

        for (auto& triangle : block_model.unoccluded_triangles)
        {
            for (auto& vertex : triangle)
                vertex.y /= 2;
        }
    }
}
