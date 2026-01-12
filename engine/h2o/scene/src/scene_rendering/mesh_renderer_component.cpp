#include "scene_rendering/mesh_renderer_component.h"

#include "core/engine.h"
#include "rendering/renderer.h"
#include "rendering/buffer.h"
#include "rendering/vertex_array.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"

namespace h2o
{
    MeshRendererComponent::MeshRendererComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
    {
        if (const auto render_system = scene.get_system<RenderingSystem>())
            render_system->register_component(*this);
    }

    MeshRendererComponent::~MeshRendererComponent()
    {
        if (const auto render_system = scene.get_system<RenderingSystem>())
            render_system->unregister_component(*this);
    }

    void MeshRendererComponent::set_mesh(const std::shared_ptr<gfx::Mesh>& mesh)
    {
        m_mesh = mesh;
    }

    void MeshRendererComponent::set_texture(const std::shared_ptr<gfx::Texture>& texture)
    {
        m_texture = texture;
    }
}