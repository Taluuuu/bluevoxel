#pragma once

#include "chunk_generator_base.h"
#include "core/resources.h"
#include "graph/graph.h"

namespace h2o
{
     class ChunkGenerator_Terrain : public ChunkGenerator_Base
     {
     public:

         ChunkGenerator_Terrain();

         // ChunkGenerator_Base interface
         void gen_blocks(Chunk::ViewType& region_view) const override;
         [[nodiscard]] std::vector<VoxelStructureInstance> gen_structures(const Chunk::ViewType& region_view) const override;
         void save(const fs::path& path) const override;
         bool load(const fs::path& path) override;

     public:

         Graph m_graph;

     };
}
