#pragma once

#include "core/handle_types.h"
#include "scene/scene_system.h"
#include "core/events.h"

#include <memory>
#include <vector>
#include <entt/entity/entity.hpp>

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

        [[nodiscard]] CameraComp* find_camera() const;

    private:

        void on_mesh_renderer_created(entt::entity entity) const;

    private:

        std::shared_ptr<gfx::IPipeline> m_pipeline = nullptr;

        gfx::IRenderer& m_renderer;

        EventHandle m_resize_event_handle{};
        f32 m_aspect_ratio = 1.0f;

    };
}
