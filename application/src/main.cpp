#include "core/engine.h"
#include "rendering/rendering_module.h"
#include "windowing/windowing_module.h"

int main()
{
    engine::Engine engine(
        engine::AppInfo
        {
            .app_name = "haaaaa"
        }
    );

    engine
        .add_module<engine::WindowingModule>()
        .add_module<engine::RenderingModule>();

    engine.run();
}