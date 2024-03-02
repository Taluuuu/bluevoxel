#pragma once

namespace h2o
{
    class BlockType;
    class UncookedBlockModel;
    class VoxelPack;
}

namespace bluevoxel
{
    class IBlockModelSelection
    {
    public:

        virtual ~IBlockModelSelection() = default;

        // Returns true if the block model was edited
        virtual bool update(
            h2o::UncookedBlockModel& block_model,
            const h2o::BlockType& block_type,
            const h2o::VoxelPack& voxel_pack) = 0;

    };
}