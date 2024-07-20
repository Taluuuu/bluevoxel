#pragma once

#include "chunk_generator_base.h"
#include "voxel/block.h"

// Needed for SmartNode caching
#include <FastNoise/FastNoise.h>

namespace h2o
{
    // class ChunkGenerator_Terrain : public ChunkGenerator_Base
    // {
    // public:
    //
    //     ChunkGenerator_Terrain();
    //     ~ChunkGenerator_Terrain() override = default;
    //
    //     // ChunkGenerator_Base interface
    //     void run_generation_step(ChunkColumn& chunk_column) const override;
    //     [[nodiscard]] i32 max_generation_stage() const override { return 1; }
    //
    // public:
    //
    //     std::vector<Block> block_layers{};
    //
    // private:
    //
    //     FastNoise::SmartNode<> m_noise_generator = nullptr;
    //
    // };
}