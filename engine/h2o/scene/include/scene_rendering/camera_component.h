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

        [[nodiscard]] const gfx::Camera& camera() const;
        void set_as_main_camera() const;

    public:

        // The FOV is multiplied by this
        f32 target_fov_modifier = 1.0f;
        f32 fov_modifier_mix_coeff = 15.0f;

    protected:

        // Tickable interface
        void update(f32 delta_time) override;
        void pre_render() override;

        [[nodiscard]] virtual v3 camera_location() const;
        [[nodiscard]] virtual v3 camera_rotation() const;

    private:

        f32 m_initial_fov = 90.0f;
        f32 m_current_fov_modifier = 1.0f;
        OwningHandle<gfx::Camera> m_camera = nullptr;

    };

    struct CameraComp
    {
        gfx::Camera camera;
    };
}