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

    constexpr i32 non_stupid_mod(const i32 x, const i32 m)
    {
        i32 r = x % m;
        return r + m * (r < 0);
    }

    // https://stackoverflow.com/questions/15014096/c-index-of-type-during-variadic-template-expansion
    template<class Target, class ListHead, class... ListTails>
    [[nodiscard]] constexpr size_t index_in_template_list()
    {
        if constexpr (std::is_same_v<Target, ListHead>)
        {
            return 0;
        }
        else
        {
            return 1 + index_in_template_list<Target, ListTails...>();
        }
    }
}