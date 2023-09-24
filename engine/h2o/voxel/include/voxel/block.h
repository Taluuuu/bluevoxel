#pragma once

#include "core/types.h"

#include <string>
#include <vector>

namespace h2o
{
    using BlockID = u16;

    struct BlockType
    {
        std::string name{};
        std::string model_name{};
        std::string block_preset_name{};
        std::vector<std::string> texture_names{};
        BlockID block_id{};
    };

    struct Block
    {
        u16 id = 0;
        u16 data_1 = 0;
        u32 data_2 = 0;

        Block() : Block(0) {}
        Block(u16 id) : id { id } {}

        static const Block Air;

        bool operator==(Block other) const
        { return id == other.id && data_1 == other.data_1 && data_2 == other.data_2; }

        template<typename S>
        void serialize(S& s)
        {
            s(id, data_1, data_2);
        }
    };
}