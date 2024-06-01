#include "bluevoxel_client_module.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "networking/networking_module.h"
#include "rendering/rendering_module.h"
#include "scene/scene_module.h"
#include "scene_rendering/scene_rendering_module.h"
#include "ui/ui_module.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "windowing/windowing_module.h"

int main()
{
    h2o::Engine engine { { "BlueVoxel Client" } };

    engine
        .add_module<h2o::VoxelModule>()
        .add_module<h2o::NetworkingModule>()
        .add_module<h2o::WindowingModule>()
        .add_module<h2o::RenderingModule>()
        .add_module<h2o::UIModule>()
        .add_module<h2o::InputModule>()
        .add_module<h2o::SceneModule>()
        .add_module<h2o::SceneRenderingModule>()
        .add_module<bluevoxel::BlueVoxelClientModule>()
        .run();
}