#pragma once

#include "core/types.h"

namespace h2o::physics
{
    struct Ray
    {
        v3 origin{};
        v3 direction{};
    };
}