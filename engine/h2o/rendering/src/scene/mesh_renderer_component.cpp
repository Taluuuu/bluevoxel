#include "rendering/scene/mesh_renderer_component.h"

#include "scene/scene.h"
#include "rendering/scene/rendering_scene_system.h"

namespace h2o
{
    MeshRendererComponent::MeshRendererComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
    {
        // Register to rendering system
        auto render_system = m_scene->get_system<RenderingSceneSystem>();
        if (render_system)
            render_system->register_component(*this);
    }

    MeshRendererComponent::~MeshRendererComponent()
    {
        // Register to rendering system
        auto render_system = m_scene->get_system<RenderingSceneSystem>();
        if (render_system)
            render_system->unregister_component(*this);
    }
}