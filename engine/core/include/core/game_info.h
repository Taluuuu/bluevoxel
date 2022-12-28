#pragma once

#include <string_view>

namespace engine
{
    struct GameInfo
    {
        std::string_view game_name;
        std::string_view engine_name;
    };
}