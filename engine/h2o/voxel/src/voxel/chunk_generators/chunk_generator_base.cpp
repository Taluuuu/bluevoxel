#include "voxel/chunk_generators/chunk_generator_base.h"

namespace h2o
{
    ChunkGenerator_Base::ChunkGenerator_Base()
    {
        recreate_seed();
    }

    void ChunkGenerator_Base::recreate_seed()
    {
        m_seed = time(nullptr);
    }
}
