#pragma once

#include "core/types.h"

namespace h2o::physics
{
    struct Ray
    {
        v3 origin{};
        v3 direction{};

        [[nodiscard]] v3 point_at(f32 t) const
        { return origin + direction * t; }
    };
}