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

        constexpr const v3& operator[](size_t index) const
        {
            switch (index)
            {
            case 0: return p1;
            case 1: return p2;
            case 2: return p3;
            default:
                assert(false);
                return p1;
            }
        }
    };

    struct Sphere
    {
        v3 center{};
        f32 radius{};
    };
}