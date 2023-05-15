#pragma once

#include "core/types.h"

#include <magic_enum.hpp>

namespace h2o::voxel
{
    enum Direction : u32
    {
        XNeg = 0, XPos,
        YNeg, YPos,
        ZNeg, ZPos,
    };

    constexpr size_t dir_count = magic_enum::enum_count<Direction>();

    constexpr v3i to_vec(u32 direction)
    {
        switch (direction)
        {
        case XNeg: return { -1,  0,  0  };
        case XPos: return {  1,  0,  0  };
        case YNeg: return {  0, -1,  0  };
        case YPos: return {  0,  1,  0  };
        case ZNeg: return {  0,  0, -1  };
        case ZPos: return {  0,  0,  1  };
        default:   assert(false);
        }
    }
}