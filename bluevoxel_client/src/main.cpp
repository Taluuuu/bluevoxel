#include "core/engine.h"
#include "voxel_client/voxel_client_module.h"

int main()
{
    h2o::Engine engine { { "BlueVoxel Client" } };

    engine
        .add_module<h2o::VoxelClientModule>()
        .run();
}