#include "scene_rendering/mesh_renderer_component.h"

#include "core/engine.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/buffer.h"
#include "rendering/vertex_array.h"

namespace h2o
{
    MeshRendererComponent::MeshRendererComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
    {
        // Register to rendering system
        auto render_system = m_scene->get_system<RenderingSceneSystem>();
        if (render_system)
            render_system->register_component(*this);

        // Temporary hard-coded triangle
        auto rendering_module = g_engine->get_module<h2o::RenderingModule>();
        assert(rendering_module);
        auto renderer = &rendering_module->renderer();

        auto buffer = renderer->create_buffer();
        vao = renderer->create_vertex_array();

        if (!buffer || !vao)
            return;

        const f32 vertices[] {
            0.0f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f, 1.0f,
            0.0f,  0.5f,  0.0f,    0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,
        };

        buffer->update_data(vertices, sizeof(vertices));
        vao->attach_vertex_buffer(buffer, 0, 0, 7 * sizeof(f32));
        vao->setup_attribute(0, 0, 3, 0);
        vao->setup_attribute(1, 0, 4, 3 * sizeof(f32));
    }

    MeshRendererComponent::~MeshRendererComponent()
    {
        // Register to rendering system
        auto render_system = m_scene->get_system<RenderingSceneSystem>();
        if (render_system)
            render_system->unregister_component(*this);
    }
}