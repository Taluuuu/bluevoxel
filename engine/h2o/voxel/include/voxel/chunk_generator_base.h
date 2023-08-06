#pragma once

#include "voxel/chunk_types.h"

namespace h2o
{
    class Chunk;
    class ChunkSystem;

    class ChunkGenerator_Base
    {
    public:

        explicit ChunkGenerator_Base(const ChunkSystem& chunk_system);
        virtual ~ChunkGenerator_Base() = default;

        virtual void gen_chunk(ChunkColumn& chunk_column) = 0;

    protected:

        [[nodiscard]] const ChunkSystem& chunk_system() const;

    private:

        const ChunkSystem* m_chunk_system = nullptr;

    };
}