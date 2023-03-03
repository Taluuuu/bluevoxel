#include "core/engine.h"
#include "game_module.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/pipeline.h"
#include "windowing/windowing_module.h"

int main()
{
    h2o::Engine engine(
        h2o::GameInfo
        {
            .game_name = "haaaaa",
            .engine_name = "hbbbbbbb"
        }
    );

    engine
        .add_module<h2o::WindowingModule>()
        .add_module<h2o::RenderingModule>()
        .add_module<game::GameModule>()
        .run();
}