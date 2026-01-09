#pragma once

#include <format>
#include <print>

namespace h2o::log
{
    template<typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args)
    {
        // We format the user's message first, then print it with the prefix
        std::println("[Error] {}", std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args)
    {
        std::println("[Warn]  {}", std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args)
    {
        std::println("[Info]  {}", std::format(fmt, std::forward<Args>(args)...));
    }
}