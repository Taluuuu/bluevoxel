#include "voxel/chunk_generators/chunk_generator_base.h"

#include <cassert>

namespace h2o
{
    ChunkGenerator_Base::ChunkGenerator_Base(const ChunkSystem& chunk_system)
        : m_chunk_system(&chunk_system)
    {}

    const ChunkSystem& ChunkGenerator_Base::chunk_system() const
    {
        assert(m_chunk_system);
        return *m_chunk_system;
    }
}