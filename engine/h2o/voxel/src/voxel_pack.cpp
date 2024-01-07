#include "voxel/voxel_pack.h"

#include <filesystem>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

namespace h2o
{
    bool VoxelPack::load(const std::string& path)
    {
        const fs::path voxel_pack_path { path };

        const fs::path block_types_path = voxel_pack_path / block_types_file_name;
        if (!exists(block_types_path))
        {
            log::error("Block types path at '{}' does not exist.", block_types_path.string());
            return false;
        }

        const fs::path block_models_path = voxel_pack_path / block_models_file_name;
        if (!exists(block_models_path))
        {
            log::error("Block models path at '{}' does not exist.", block_models_path.string());
            return false;
        }

        const fs::path textures_path = voxel_pack_path / textures_folder_name;
        if (!exists(textures_path))
        {
            log::error("Voxel textures folder path at '{}' does not exist.", textures_path.string());
            return false;
        }



        return true;
    }
}