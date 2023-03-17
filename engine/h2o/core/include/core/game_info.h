#pragma once

#include <string_view>

namespace h2o
{
    // This should be a struct, but it won't compile under MSVC if it is. No fucking clue why.
    class GameInfo
    {
    public:

        std::string_view game_name;
        std::string_view engine_name;

    };
}