#pragma once

#include "core/types.h"

namespace h2o
{
    class ChunkColumn;
    class ChunkRegion;
    class ChunkSystem;

    class ChunkGenerator_Base
    {
    public:

        virtual ~ChunkGenerator_Base() = default;

        virtual void run_generation_step(const ChunkRegion& chunk_region) const = 0;
        [[nodiscard]] virtual i32 max_generation_stage() const = 0;

    };
}