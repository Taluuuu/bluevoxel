#include "game_framework/actors/triangle_actor.h"

#include "core/engine.h"
#include "rendering/rendering_module.h"

namespace h2o
{
    TriangleActor::TriangleActor(const ActorInitializer& actor_initializer) : Actor(actor_initializer)
    {
        g_engine->register_tickable(this, TickPhase::Render);

        //auto rendering_module = g_engine->get_module<h2o::RenderingModule>();
        //assert(rendering_module);
        //m_renderer = &rendering_module->renderer();
        //
        //m_pipeline = (*m_renderer)
        //    .create_pipeline()
        //    .add_shader(h2o::gfx::ShaderStage::Vertex,   "Resources/engine/shaders/opengl/triangle.vert")
        //    .add_shader(h2o::gfx::ShaderStage::Fragment, "Resources/engine/shaders/opengl/triangle.frag")
        //    .compile();
        //
        //auto buffer = m_renderer->create_buffer();
        //m_vertex_array = m_renderer->create_vertex_array();
        //
        //if (!m_pipeline || !m_vertex_array || !buffer)
        //    return false;
        //
        //const f32 vertices[] {
        //    0.0f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f, 1.0f,
        //    0.0f,  0.5f,  0.0f,    0.0f, 1.0f, 0.0f, 1.0f,
        //    0.0f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,
        //};
        //
        //buffer->update_data(vertices, sizeof(vertices));
        //m_vertex_array->attach_vertex_buffer(buffer, 0, 0, 7 * sizeof(f32));
        //m_vertex_array->setup_attribute(0, 0, 3, 0);
        //m_vertex_array->setup_attribute(1, 0, 4, 3 * sizeof(f32));

    }

    void TriangleActor::tick(TickPhase phase, f64 delta_time)
    {

    }
}