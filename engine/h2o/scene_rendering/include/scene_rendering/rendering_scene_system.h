#pragma once

#include "core/handle_types.h"
#include "scene/scene_system.h"
#include "core/events.h"

#include <memory>
#include <vector>

namespace h2o
{
    namespace gfx
    {
        class Camera;
        class IPipeline;
        class IRenderer;
    }

    class MeshRendererComponent;

    class RenderingSystem : public SceneSystem
    {
    public:

        explicit RenderingSystem(const SceneSystemInitializer& system_initializer);
        ~RenderingSystem() override;

        // Tickable interface
        void pre_render() override;
        void render() override;

        // Registered mesh renderer components MUST unregister themselves on delete
        void register_component(const MeshRendererComponent& renderer_component);
        void unregister_component(const MeshRendererComponent& renderer_component);

        void set_main_camera(const WeakHandle<gfx::Camera>& camera);
        [[nodiscard]] const WeakHandle<gfx::Camera>& main_camera() const { return m_main_camera; }

    private:

        std::vector<const MeshRendererComponent*> m_mesh_renderer_components;

        WeakHandle<gfx::Camera> m_main_camera = nullptr;
        std::shared_ptr<h2o::gfx::IPipeline> m_pipeline = nullptr;

        gfx::IRenderer* m_renderer = nullptr;

        h2o::EventHandle m_resize_event_handle;
        f32 m_aspect_ratio = 1.0f;

    };
}