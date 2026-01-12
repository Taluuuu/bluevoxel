#pragma once

#include "scene_rendering/camera_component.h"

namespace h2o
{
    class InputModule;

    class FpsCameraComponent : public CameraComponent
    {
    public:

        explicit FpsCameraComponent(const ComponentInitializer& component_initializer);
        ~FpsCameraComponent() override = default;

    protected:

        void update(f32 delta_time) override;

    private:

        InputModule& m_input_module;

    };
}