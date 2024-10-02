#pragma once

#include "core/types.h"

#include <functional>
#include <magic_enum.hpp>

namespace h2o::voxel
{
    namespace Direction
    {
        enum Type : u8
        {
            XNeg = 1 << 0,
            XPos = 1 << 1,
            ZNeg = 1 << 2,
            ZPos = 1 << 3,
            YNeg = 1 << 4,
            YPos = 1 << 5,
        };

        inline constexpr Type None = static_cast<Type>(0);
        inline constexpr Type All = static_cast<Type>(XNeg | XPos | ZNeg | ZPos | YNeg | YPos);

        constexpr void for_each(const std::function<void(Type)>& function)
        {
            for (Type dir : { XNeg, XPos, ZNeg, ZPos, YNeg, YPos })
                function(dir);
        }
    }

    enum class Axis
    {
        X, Y, Z
    };

    constexpr u8 to_index(const Direction::Type dir)
    {
        switch (dir)
        {
        case Direction::XNeg: return 0;
        case Direction::XPos: return 1;
        case Direction::ZNeg: return 2;
        case Direction::ZPos: return 3;
        case Direction::YNeg: return 4;
        case Direction::YPos: return 5;
        default: return Direction::None;
        }
    }

    constexpr Direction::Type to_direction(const u8 index)
    {
        switch (index)
        {
        case 0: return Direction::XNeg;
        case 1: return Direction::XPos;
        case 2: return Direction::ZNeg;
        case 3: return Direction::ZPos;
        case 4: return Direction::YNeg;
        case 5: return Direction::YPos;
        default: return Direction::None;
        }
    }

    constexpr Direction::Type to_direction(Axis axis)
    {
        switch (axis)
        {
        case Axis::X: return Direction::XPos;
        case Axis::Y: return Direction::YPos;
        case Axis::Z: return Direction::ZPos;
        default: return Direction::None;
        }
    }

    constexpr v3i to_vec3(Direction::Type direction)
    {
        switch (direction)
        {
        case Direction::XNeg: return { -1,  0,  0  };
        case Direction::XPos: return {  1,  0,  0  };
        case Direction::ZNeg: return {  0,  0, -1  };
        case Direction::ZPos: return {  0,  0,  1  };
        case Direction::YNeg: return {  0, -1,  0  };
        case Direction::YPos: return {  0,  1,  0  };
        default:   return {  0,  0,  0  };
        }
    }

    constexpr v2i to_vec2(Direction::Type direction)
    {
        switch (direction)
        {
        case Direction::XNeg: return { -1,  0  };
        case Direction::XPos: return {  1,  0  };
        case Direction::ZNeg: return {  0, -1  };
        case Direction::ZPos: return {  0,  1  };
        default:   return {  0,  0  };
        }
    }

    constexpr Direction::Type invert(Direction::Type direction)
    {
        switch (direction)
        {
        case Direction::XNeg: return Direction::XPos;
        case Direction::XPos: return Direction::XNeg;
        case Direction::ZNeg: return Direction::ZPos;
        case Direction::ZPos: return Direction::ZNeg;
        case Direction::YNeg: return Direction::YPos;
        default:
        case Direction::YPos: return Direction::YNeg;
        }
    }
}