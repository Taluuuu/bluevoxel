#include "bluevoxel_server_module.h"
#include "core/engine.h"
#include "networking/networking_module.h"
#include "scene/scene_module.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"

int main()
{
    h2o::Engine engine { { "BlueVoxel Server" } };

    engine
        .add_module<h2o::VoxelModule>()
        .add_module<h2o::NetworkingModule>()
        .add_module<h2o::SceneModule>()
        .add_module<bluevoxel::BlueVoxelServerModule>()
        .run();
}