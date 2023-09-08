#include "voxel_client/voxel_client_module.h"

#include "voxel/voxel_module.h"

namespace h2o
{
    std::string_view h2o::VoxelClientModule::module_name() const
    {
        return "h2o_voxel_client";
    }

    std::vector<std::type_index> VoxelClientModule::dependencies() const
    {
        return { typeid(VoxelModule) };
    }
}