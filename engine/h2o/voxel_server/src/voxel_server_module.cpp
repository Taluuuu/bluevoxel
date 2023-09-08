#include "voxel_server/voxel_server_module.h"

#include "voxel/voxel_module.h"

namespace h2o
{
    std::string_view VoxelServerModule::module_name() const
    {
        return "h2o_voxel_server";
    }

    std::vector<std::type_index> VoxelServerModule::dependencies() const
    {
        return { typeid(VoxelModule) };
    }
}