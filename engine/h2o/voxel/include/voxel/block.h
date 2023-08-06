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

        Block(u64 id) : id{id}, data{0} {}
        Block(u64 id, u64 data) : id{id}, data{data} {}

        static const Block Air;

        bool operator==(Block other) const
        { return id == other.id && data == other.data; }
    };
}