#pragma once

#include "scene/component.h"

namespace h2o
{
    namespace gfx
    {
        class Camera;
    }

    class CameraComponent : public Component
    {
    public:

        explicit CameraComponent(const ComponentInitializer& component_initializer);
        ~CameraComponent() override = default;

        // Component interface
        void pre_render() override;

        [[nodiscard]] const gfx::Camera& camera() const;
        void set_as_main_camera() const;

    private:

        OwningHandle<gfx::Camera> m_camera;

    };
}