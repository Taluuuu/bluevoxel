#pragma once

#include "core/types.h"

#include <string>
#include <vector>

namespace h2o
{
    using BlockID = u16;

    struct Block
    {
        BlockID id{};
        u16 data{};

        Block() : Block(0) {}
        constexpr Block(BlockID id) : id { id } {}

        static const Block Air;

        bool operator==(Block other) const
        {
            return id == other.id;
        }

        template<typename S>
        void serialize(S& s)
        {
            s(id);
        }
    };
}