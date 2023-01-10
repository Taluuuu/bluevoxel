#pragma once

#include <fmt/color.h>
#include <fmt/core.h>

namespace engine::log
{
    /**
     * @brief Log an error message
     * 
     * @param format_str An fmt formatted string
     * @param args Message arguments
     */
    template<typename S, typename... Args>
    void error(const S& format_str, const Args&... args)
    {
        auto prefixed = fmt::format("[Error] {}\n", format_str);
        fmt::print(fg(fmt::color::red), prefixed, args...);
    }

    /**
     * @brief Log a warning
     * 
     * @param format_str An fmt formatted string
     * @param args Message arguments
     */
    template<typename S, typename... Args>
    void warn(const S& format_str, const Args&... args)
    {
        auto prefixed = fmt::format("[Warn] {}\n", format_str);
        fmt::print(fg(fmt::color::yellow), prefixed, args...);
    }
    
    /**
     * @brief Log an info message
     * 
     * @param format_str An fmt formatted string
     * @param args Message arguments
     */
    template<typename S, typename... Args>
    void info(const S& format_str, const Args&... args)
    {
        auto prefixed = fmt::format("[Info] {}\n", format_str);
        fmt::print(fg(fmt::color::white), prefixed, args...);
    }
}