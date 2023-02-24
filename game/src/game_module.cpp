#include "game_module.h"

#include "core/engine.h"
#include "core/log.h"
#include "rendering/pipeline.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"

namespace game
{
    GameModule::GameModule(engine::Engine& engine)
        : engine::Module(engine)
    {}

    bool GameModule::init(const engine::GameInfo& game_info)
    {
        if (!engine::Module::init(game_info))
            return false;

        auto rendering_module = m_engine->get_module<engine::RenderingModule>();
        assert(rendering_module);
        auto& renderer = rendering_module->renderer();

        auto pipeline = renderer
            .create_pipeline()
            .add_shader(engine::ShaderStage::Vertex,   "Resources/engine/shaders/opengl/triangle.vert")
            .add_shader(engine::ShaderStage::Fragment, "Resources/engine/shaders/opengl/triangle.frag")
            .compile();

        if (!pipeline)
            return false;

        renderer.bind_pipeline(pipeline);

        return true;
    }

    std::string_view GameModule::get_module_name() const
    {
        return "GameModule";
    }

    std::vector<std::type_index> GameModule::get_dependencies() const
    {
        return { typeid(engine::RenderingModule) };
    }

    void GameModule::tick(f64 delta_time)
    {

    }
}