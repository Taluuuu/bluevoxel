#include "core/engine.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/pipeline.h"
#include "windowing/windowing_module.h"

int main()
{
    engine::Engine engine(
        engine::GameInfo
        {
            .game_name = "haaaaa",
            .engine_name = "hbbbbbbb"
        }
    );

    engine
        .add_module<engine::WindowingModule>()
        .add_module<engine::RenderingModule>()
        .run();
}