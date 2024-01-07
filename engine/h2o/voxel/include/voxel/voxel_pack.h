#pragma once

#include "core/resources.h"

#include <string_view>

namespace h2o
{
    class VoxelPack : public IResource
    {
    public:

        // IResource interface
        bool load(const std::string& path) override;



    protected:

        static constexpr std::string_view block_types_file_name { "block_types.yml" };
        static constexpr std::string_view block_models_file_name { "block_models.yml" };
        static constexpr std::string_view textures_folder_name { "textures" };

    };
}