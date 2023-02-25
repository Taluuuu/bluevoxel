#include "game_module.h"

#include "core/engine.h"
#include "core/log.h"
#include "rendering/pipeline.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/vertex_array.h"
#include "rendering/buffer.h"

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
        m_renderer = &rendering_module->renderer();

        m_pipeline = (*m_renderer)
            .create_pipeline()
            .add_shader(engine::ShaderStage::Vertex,   "Resources/engine/shaders/opengl/triangle.vert")
            .add_shader(engine::ShaderStage::Fragment, "Resources/engine/shaders/opengl/triangle.frag")
            .compile();

        m_vertex_array = m_renderer->create_vertex_array();
        auto buffer = m_renderer->create_buffer();

        if (!m_pipeline || !m_vertex_array || !buffer)
            return false;

        std::vector<f32> vertices = {
            -0.5f, -0.5f,
             0.5f, -0.5f,
             0.0f,  0.5f
        };

        buffer->update_data(vertices.data(), vertices.size() * sizeof(f32));
        m_vertex_array->attach_vertex_buffer(buffer, 0, 0, sizeof(f32));
        m_vertex_array->setup_attribute(0, 0, 2, 2 * sizeof(f32));

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
        assert(m_renderer && m_pipeline && m_vertex_array);

        m_renderer->bind_pipeline(m_pipeline);
        m_renderer->draw(*m_vertex_array);
    }
}