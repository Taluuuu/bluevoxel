#pragma once

#include "core/types.h"

namespace h2o::vxl
{
    struct Block
    {
        u64 id   : 16 {0};
        u64 data : 48 {0};
    };
}