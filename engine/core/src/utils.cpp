#include "core/utils.h"

#include <fstream>

namespace engine::utils
{
    std::optional<std::vector<char>> read_file(const std::string& path)
    {
        std::ifstream file(path.data(), std::ios::ate | std::ios::binary);

        if (!file.is_open())
            return std::nullopt;

        size_t file_size = (size_t)file.tellg();
        std::vector<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();
        return buffer;
    }
}