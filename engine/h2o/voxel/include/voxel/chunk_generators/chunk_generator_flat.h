#pragma once

#include "chunk_generator_base.h"
#include "voxel/block.h"

#include <vector>

namespace h2o
{
    class ChunkGenerator_Flat : public ChunkGenerator_Base
    {
    public:

        ChunkGenerator_Flat() = default;

        // ChunkGenerator_Base interface
        void gen_blocks(ChunkView& region_view) const override;
        [[nodiscard]] std::vector<VoxelStructureInstance> gen_structures(const ChunkView& region_view) const override;

    public:

        std::vector<Block> block_layers{};

    };
}