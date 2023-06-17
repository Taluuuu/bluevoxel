#pragma once

#include "core/resources.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace h2o
{
    class VoxelPack : public IResource
    {
    public:

        // IResource interface
        bool load(const std::string& path) override;

        [[nodiscard]] const fs::path& block_types_path()  const { return m_block_types_path;  }
        [[nodiscard]] const fs::path& block_models_path() const { return m_block_models_path; }
        [[nodiscard]] const fs::path& textures_path()     const { return m_textures_path;     }

    private:

        fs::path m_block_types_path;
        fs::path m_block_models_path;
        fs::path m_textures_path;

    };
}