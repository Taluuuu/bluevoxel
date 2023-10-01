#include "bluevoxel_client_module.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "networking/networking_module.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene_module.h"
#include "scene_rendering/scene_rendering_module.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "voxel_client/voxel_client_module.h"
#include "voxel_rendering/voxel_rendering_module.h"
#include "windowing/windowing_module.h"

int main()
{
    h2o::Engine engine { { "BlueVoxel Client" } };

    const auto voxel_pack = engine
        .resource_mgr()
        .fetch<h2o::VoxelPack>("../Resources/bluevoxel_server/voxel/pack.yml");

    engine
        .add_module<h2o::VoxelModule>(voxel_pack)
        .add_module<h2o::VoxelRenderingModule>(voxel_pack)
        .add_module<h2o::VoxelClientModule>()
        .add_module<h2o::NetworkingModule>()
        .add_module<h2o::WindowingModule>()
        .add_module<h2o::RenderingModule>()
        .add_module<h2o::InputModule>()
        .add_module<h2o::SceneModule>()
        .add_module<h2o::SceneRenderingModule>()
        .add_module<bluevoxel::BlueVoxelClientModule>()
        .run();
}