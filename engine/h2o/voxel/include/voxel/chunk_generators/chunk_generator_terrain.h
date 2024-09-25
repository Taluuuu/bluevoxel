#pragma once

#include "chunk_generator_base.h"

#include <FastNoise/FastNoise.h>

namespace h2o
{
     class ChunkGenerator_Terrain : public ChunkGenerator_Base
     {
     public:

         ChunkGenerator_Terrain();

         // ChunkGenerator_Base interface
         void gen_blocks(ChunkRegionView& region_view) const override;
         [[nodiscard]] std::vector<VoxelStructureInstance> gen_structures(const ChunkRegionView& region_view) const override;

     private:

         FastNoise::SmartNode<> m_noise_generator = nullptr;

     };
}