#pragma once

#include "core/types.h"

#include <magic_enum.hpp>

namespace h2o::voxel
{
    enum Direction : u8
    {
        XNeg = 1 << 0,
        XPos = 1 << 1,
        ZNeg = 1 << 2,
        ZPos = 1 << 3,
        YNeg = 1 << 4,
        YPos = 1 << 5,
    };

    constexpr v3i to_vec3(Direction direction)
    {
        switch (direction)
        {
        case XNeg: return { -1,  0,  0  };
        case XPos: return {  1,  0,  0  };
        case ZNeg: return {  0,  0, -1  };
        case ZPos: return {  0,  0,  1  };
        case YNeg: return {  0, -1,  0  };
        case YPos: return {  0,  1,  0  };
        default:   return {  0,  0,  0  };
        }
    }

    constexpr v2i to_vec2(Direction direction)
    {
        switch (direction)
        {
        case XNeg: return { -1,  0  };
        case XPos: return {  1,  0  };
        case ZNeg: return {  0, -1  };
        case ZPos: return {  0,  1  };
        default:   return {  0,  0  };
        }
    }

    constexpr Direction invert(Direction direction)
    {
        switch (direction)
        {
        case XNeg: return XPos;
        case XPos: return XNeg;
        case ZNeg: return ZPos;
        case ZPos: return ZNeg;
        case YNeg: return YPos;
        default:
        case YPos: return YNeg;
        }
    }
}