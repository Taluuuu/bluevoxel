#pragma once

#include "core/types.h"

namespace h2o
{
    struct VoxelStructureData
    {
        v3i corner{};
        u32 structure_id = 0;
    };
}