#include "bluevoxel_server_module.h"
#include "core/engine.h"
#include "networking/networking_module.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "voxel_server/voxel_server_module.h"

int main()
{
    h2o::Engine engine { { "BlueVoxel Server" } };

    const auto voxel_pack = engine
        .resource_mgr()
        .fetch<h2o::VoxelPack>("../Resources/bluevoxel_server/voxel/pack.yml");

    engine
        .add_module<h2o::VoxelModule>(voxel_pack)
        .add_module<h2o::VoxelServerModule>()
        .add_module<h2o::NetworkingModule>()
        .add_module<bluevoxel::BlueVoxelServerModule>()
        .run();
}