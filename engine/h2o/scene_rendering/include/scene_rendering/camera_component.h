#pragma once

#include "scene/component.h"
// Could be a forward declaration but CLion linter doesn't like it
#include "rendering/camera.h"
#include "core/core_interfaces.h"

namespace h2o
{
    class CameraComponent
        : public Component
        , public ITickable
    {
    public:

        explicit CameraComponent(const ComponentInitializer& component_initializer);
        ~CameraComponent() override = default;

        // ITickable interface
        void tick(h2o::TickPhase phase, f64 delta_time) override;

        void set_as_main_camera() const;

    private:

        OwningHandle<gfx::Camera> m_camera;

    };
}