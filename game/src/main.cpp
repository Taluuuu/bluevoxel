#include "core/engine.h"
#include "game_module.h"
#include "input/input_module.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/pipeline.h"
#include "scene/scene_module.h"
#include "windowing/windowing_module.h"
#include "game_framework/game_framework_module.h"
#include "scene_rendering/scene_rendering_module.h"
#include "voxel/voxel_module.h"
#include "voxel_rendering/voxel_rendering_module.h"

int main()
{
    h2o::Engine engine(
        h2o::GameInfo
        {
            .game_name = "haaaaa"
        }
    );

    engine
        .add_module<h2o::WindowingModule>()
        .add_module<h2o::RenderingModule>()
        .add_module<h2o::InputModule>()
        .add_module<h2o::SceneModule>()
        .add_module<h2o::GameFrameworkModule>()
        .add_module<h2o::SceneRenderingModule>()
        .add_module<h2o::VoxelModule>()
        .add_module<h2o::VoxelRenderingModule>()
        .add_module<game::GameModule>()
        .run();
}