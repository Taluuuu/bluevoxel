#pragma once

#include "chunk_generator_base.h"

namespace h2o
{
    class ChunkGenerator_Sphere : public ChunkGenerator_Base
    {
    public:

        ~ChunkGenerator_Sphere() override = default;

        // ChunkGenerator_Base interface
        void run_generation_step(ChunkColumn& chunk_column) const override;
        [[nodiscard]] i32 max_generation_stage() const override { return 1; }

    };
}