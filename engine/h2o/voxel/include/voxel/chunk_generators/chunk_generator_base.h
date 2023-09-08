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

        explicit ChunkGenerator_Base(const ChunkSystem& chunk_system);
        virtual ~ChunkGenerator_Base() = default;

        virtual void run_generation_step(ChunkColumn& chunk_col, StaticChunkRegion& chunk_region) const = 0;
        virtual void run_generation_step(ChunkRegion& chunk_region) const = 0;
        [[nodiscard]] virtual i32 max_generation_stage() const = 0;

    protected:

        [[nodiscard]] const ChunkSystem& chunk_system() const;

    private:

        const ChunkSystem* m_chunk_system = nullptr;

    };
}