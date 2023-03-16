#include "game_module.h"

#include "core/engine.h"
#include "core/log.h"
#include "rendering/camera.h"
#include "rendering/pipeline.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/vertex_array.h"
#include "rendering/buffer.h"

namespace game
{
    bool GameModule::init(h2o::Engine& engine)
    {
        engine.register_tickable(this, h2o::TickPhase::Render);

        auto rendering_module = engine.get_module<h2o::RenderingModule>();
        assert(rendering_module);
        m_renderer = &rendering_module->renderer();

        m_pipeline = (*m_renderer)
            .create_pipeline()
            .add_shader(h2o::gfx::ShaderStage::Vertex,   "Resources/engine/shaders/opengl/triangle.vert")
            .add_shader(h2o::gfx::ShaderStage::Fragment, "Resources/engine/shaders/opengl/triangle.frag")
            .compile();

        auto buffer = m_renderer->create_buffer();
        m_vertex_array = m_renderer->create_vertex_array();

        if (!m_pipeline || !m_vertex_array || !buffer)
            return false;

        const f32 vertices[] {
            0.0f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f, 1.0f,
            0.0f,  0.5f,  0.0f,    0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,
        };

        buffer->update_data(vertices, sizeof(vertices));
        m_vertex_array->attach_vertex_buffer(buffer, 0, 0, 7 * sizeof(f32));
        m_vertex_array->setup_attribute(0, 0, 3, 0);
        m_vertex_array->setup_attribute(1, 0, 4, 3 * sizeof(f32));

        m_camera = std::make_shared<h2o::gfx::Camera>(90.0f, 800.0f / 600.0f);
        m_camera->update({ -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });

        m_pipeline->set_uniform_mat4(0, m_camera->proj_view());

        return true;
    }

    std::vector<std::type_index> GameModule::dependencies() const
    {
        return { typeid(h2o::RenderingModule) };
    }

    void GameModule::tick(h2o::TickPhase phase, f64 delta_time)
    {
        assert(m_renderer && m_pipeline && m_vertex_array);

        m_renderer->bind_pipeline(m_pipeline);
        m_renderer->draw(*m_vertex_array);
    }
}