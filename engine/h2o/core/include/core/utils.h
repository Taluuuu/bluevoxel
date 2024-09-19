#pragma once

#include "core/types.h"

#include <optional>
#include <string>
#include <vector>

namespace h2o::utils
{
    /**
     * @brief Read a file to a vector of characters.
     * 
     * @param path The file's path
     * @return A vector of characters or std::nullopt if the file
     *         couldn't be opened.
     */
    std::optional<std::vector<char>> read_file(const std::string& path);

    template<class T>
    bool contains(const std::vector<T>& vec, const T& val)
    { return std::ranges::find(vec.begin(), vec.end(), val) != vec.end(); }

    constexpr i32 non_stupid_mod(i32 x, i32 m)
    {
        i32 r = x % m;
        return r + m * (r < 0);
    }
}