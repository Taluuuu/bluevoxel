#pragma once

#include "core/types.h"

namespace h2o
{
    class ChunkColumn;
    class ChunkRegion;
    class ChunkSystem;
    class StaticChunkRegion;

    class ChunkGenerator_Base
    {
    public:

        virtual ~ChunkGenerator_Base() = default;

        // Deprecated
        virtual void run_generation_step(ChunkColumn& chunk_col, StaticChunkRegion& chunk_region) const {};
        virtual void run_generation_step(ChunkRegion& chunk_region) const = 0;
        [[nodiscard]] virtual i32 max_generation_stage() const = 0;

    };
}