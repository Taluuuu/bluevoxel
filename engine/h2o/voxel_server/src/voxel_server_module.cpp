#include "voxel_server/voxel_server_module.h"

#include "voxel/voxel_module.h"
#include "voxel/chunk_generators/chunk_generator_flat.h"
#include "networking/networking_module.h"

namespace h2o
{
    std::vector<std::type_index> VoxelServerModule::dependencies() const
    {
        return {
            typeid(VoxelModule),
            typeid(NetworkingModule)
        };
    }
}