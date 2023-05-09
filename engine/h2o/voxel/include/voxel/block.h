#pragma once

#include "core/types.h"

namespace h2o
{
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