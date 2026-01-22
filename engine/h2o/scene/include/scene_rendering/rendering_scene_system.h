#pragma once

#include "core/handle_types.h"
#include "scene/scene_system.h"
#include "core/events.h"

#include <memory>
#include <vector>

namespace h2o
{
    struct CameraComp;
    class MeshRendererComponent;

    namespace gfx
    {
        class Camera;
        class IPipeline;
        class IRenderer;
    }

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

        [[nodiscard]] CameraComp* find_camera() const;

    private:

        std::vector<const MeshRendererComponent*> m_mesh_renderer_components{};

        std::shared_ptr<gfx::IPipeline> m_pipeline = nullptr;

        gfx::IRenderer& m_renderer;

        EventHandle m_resize_event_handle{};
        f32 m_aspect_ratio = 1.0f;

    };
}