#pragma once

#include "core/types.h"

namespace h2o
{
    using BlockID = u16;

    constexpr u32 max_data_bits = 16;

    struct Block
    {
        BlockID id{};
        u16 data{};

        constexpr Block(const BlockID id = 0) : id { id } {}
        constexpr Block(const BlockID id, const u16 data) : id { id }, data { data } {}

        static const Block Air;

        bool operator==(const Block other) const
        {
            return id == other.id && data == other.data;
        }

        template<typename S>
        void serialize(S& s)
        {
            s(id, data);
        }
    };
}