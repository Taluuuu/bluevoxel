#include "core/engine.h"
#include "rendering/renderer.h"
#include "rendering/renderer_enums.h"
#include "rendering/rendering_module.h"
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

    auto rendering_module = engine.get_module<engine::RenderingModule>();
    if (rendering_module)
    {
        auto renderer = rendering_module->renderer();

        auto pipeline = renderer
            .create_pipeline()
            .add_shader(engine::ShaderType::Vertex,   "Resources/engine/shaders/triangle.vert.spv")
            .add_shader(engine::ShaderType::Fragment, "Resources/engine/shaders/triangle.frag.spv")
            .compile();

        if (pipeline.is_ready())
        {
            renderer.use_pipeline(pipeline);
        }
        else
        {
            // Failure
        }
    }
}