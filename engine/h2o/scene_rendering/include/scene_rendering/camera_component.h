#pragma once

#include "rendering/camera.h" // TODO: Remove this include
#include "scene/component.h"

namespace h2o
{
    class CameraComponent : public Component
    {
    public:

        explicit CameraComponent(const ComponentInitializer& component_initializer);
        ~CameraComponent() override = default;

        // Component interface
        void pre_render() override;

        [[nodiscard]] const gfx::Camera& camera() const;
        void set_as_main_camera() const;

    protected:

        [[nodiscard]] virtual v3 camera_location() const;
        [[nodiscard]] virtual v3 camera_rotation() const;

    private:

        OwningHandle<gfx::Camera> m_camera;

    };
}