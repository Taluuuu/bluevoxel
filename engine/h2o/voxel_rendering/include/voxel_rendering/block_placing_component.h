#pragma once

#include "scene/component.h"

namespace h2o
{
    class CameraComponent;
    class InputComponent;

    // This module is a weird place to put this, but it will work fine for now
    class BlockPlacingComponent : public Component
    {
    public:

        explicit BlockPlacingComponent(const ComponentInitializer& component_initializer);
        ~BlockPlacingComponent() override = default;

        void update(f32 delta_time) override;

    private:

        WeakHandle<CameraComponent> m_camera { nullptr };
        WeakHandle<InputComponent> m_input { nullptr };

    };
}