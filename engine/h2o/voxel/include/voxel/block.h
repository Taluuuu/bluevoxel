#pragma once

#include "core/types.h"

#include <string>
#include <vector>

namespace h2o
{
    using BlockID = u16;

    struct BlockType
    {
        std::string name;
        std::string model_name;
        std::vector<std::string> texture_names;
    };

    struct Block
    {
        u64 id   : 16 { 0 };
        u64 data : 48 { 0 };

        static const Block Air;
    };

    inline bool operator==(Block lhs, Block rhs)
    {
        return lhs.id == rhs.id && lhs.data == rhs.data;
    }
}