#pragma once

#include "scene_rendering/camera_component.h"

namespace h2o
{
    class InputComponent;

    class ThirdPersonCameraComponent : public CameraComponent
    {
    public:

        explicit ThirdPersonCameraComponent(const ComponentInitializer& component_initializer);
        ~ThirdPersonCameraComponent() override = default;

    public:

        f32 distance_with_actor = 5.0f;
        f32 scroll_zoom_factor = 0.1f;

    protected:

        // Tickable interface
        void update(f32 delta_time) override;

        // CameraComponent interface
        [[nodiscard]] v3 camera_location() const override;

    private:

        WeakHandle<InputComponent> m_input = nullptr;

    };
}