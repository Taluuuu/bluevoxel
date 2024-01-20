#pragma once

#include "core/types.h"

namespace h2o::physics
{
    struct Cylinder
    {
        v3 p1{}, p2{};
        f32 radius = 0.0f;
    };

    struct Plane
    {
        v3 point{};
        v3 normal{};
    };

    struct Disk
    {
        v3 center{};
        v3 normal{};
        f32 radius = 0.0f;
    };

    struct Triangle
    {
        v3 p1{}, p2{}, p3{};
    };
}