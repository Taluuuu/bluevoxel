#include "voxel/chunk.h"

namespace h2o
{
    Chunk::Chunk(size_t size)
        : m_size(size)
    {
        m_blocks.reserve(volume());
    }
}