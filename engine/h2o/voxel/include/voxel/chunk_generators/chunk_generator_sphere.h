#pragma once

#include "chunk_generator_base.h"
#include "voxel/block.h"
#include "core/types.h"

#include <vector>

namespace h2o
{
    class ChunkGenerator_Sphere : public ChunkGenerator_Base
    {
    public:

        ~ChunkGenerator_Sphere() override = default;

        // ChunkGenerator_Base interface
        void run_generation_step(ChunkColumn& chunk_col, StaticChunkRegion& chunk_region) const override;
        void run_generation_step(ChunkRegion& chunk_region) const override;
        [[nodiscard]] i32 max_generation_stage() const override { return 1; }

    };
}