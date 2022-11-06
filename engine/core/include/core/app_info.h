#pragma once

#include <string_view>

namespace engine
{
    struct AppInfo
    {
        std::string_view app_name;
        std::string_view engine_name;
    };
}