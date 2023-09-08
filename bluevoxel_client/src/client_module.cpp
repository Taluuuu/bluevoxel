#include "client_module.h"
#include "voxel_client/voxel_client_module.h"

namespace client
{
    std::vector<std::type_index> ClientModule::dependencies() const
    {
        return { typeid(h2o::VoxelClientModule) };
    }
}