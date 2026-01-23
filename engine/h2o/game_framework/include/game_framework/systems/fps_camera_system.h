#pragma once

#include "scene/scene_system.h"

namespace h2o
{
    class InputModule;

    class FpsCameraSystem : public SceneSystem
    {
    public:

        explicit FpsCameraSystem(const SceneSystemInitializer& system_initializer);

        void update(f32 delta_time) override;

    private:

        InputModule& m_input_module;

    };
}
