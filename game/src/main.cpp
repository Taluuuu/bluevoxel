#include "core/engine.h"
#include "rendering/pipeline.h"
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

    // auto rendering_module = engine.get_module<engine::RenderingModule>();
    // if (rendering_module)
    // {
    //     auto& renderer = rendering_module->renderer();
    //     auto& pipeline = renderer.create_pipeline()
    //         .add_shader(engine::ShaderStage::Vertex,   "shaders/triangle.vert")
    //         .add_shader(engine::ShaderStage::Fragment, "shaders/triangle.frag")
    //         .compile();

    //     if (pipeline.is_ready())
    //     {
            
    //     }
    // }
}