#pragma once

#include "core/types.h"

#include <optional>
#include <string>
#include <vector>

namespace engine::utils
{
    /**
     * @brief Read a file to a vector of characters.
     * 
     * @param path The file's path
     * @return A vector of characters or std::nullopt if the file
     *         couldn't be opened.
     */
    std::optional<std::vector<char>> read_file(const std::string& path);
}