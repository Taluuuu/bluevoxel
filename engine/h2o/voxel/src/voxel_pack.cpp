#include "voxel/voxel_pack.h"

#include <yaml-cpp/yaml.h>

namespace h2o
{
    bool VoxelPack::load(const std::string& path)
    {
        try
        {
            auto parent_path = fs::path{ path }.parent_path();
            if (parent_path.empty())
            {
                log::error("Invalid voxel pack path: {}", path);
                return false;
            }

            const auto root = YAML::LoadFile(path);

            m_block_types_path = parent_path / root["block_types_path"].as<std::string>();
            if (!exists(m_block_types_path))
                log::warn("Block types path at '{}' does not exist.", m_block_types_path.string());

            m_block_models_path = parent_path / root["block_models_path"].as<std::string>();
            if (!exists(m_block_models_path))
                log::warn("Block models path at '{}' does not exist.", m_block_models_path.string());

            m_textures_path = parent_path / root["textures_path"].as<std::string>();
            if (!exists(m_textures_path))
                log::warn("Voxel textures folder path at '{}' does not exist.", m_textures_path.string());
        }
        catch (const std::exception& e)
        {
            log::error("Failed to load voxel pack at path: {}. Error: {}", path, e.what());
            return false;
        }

        return true;
    }
}