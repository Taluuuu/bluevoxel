#pragma once

#include "core/types.h"
#include "block.h"

#include <string>
#include <vector>

namespace h2o
{
    struct BlockType
    {
        std::string name{};
        BlockID block_id = 0;

        std::vector<u32> texture_ids{};
        u32 model_id = 0;
        u32 preset_id = 0;
    };
}