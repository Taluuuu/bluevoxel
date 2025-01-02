#pragma once

#include "core/events.h"
#include "scene/scene_system.h"

namespace h2o
{
    class WeatherSystem : public SceneSystem
    {
    public:

        explicit WeatherSystem(const SceneSystemInitializer& system_initializer);

        // Tickable interface
        void network_update(f32 delta_time) override;
        void render() override;

    public:

        f32 sun_distance = 300.0f;
        f32 sun_radius = 30.0f;
        v4 sun_color{ 0.98f, 0.95f, 0.82f, 1.0f };

        f32 world_brightness = 0.0f;

        struct LightingData
        {
            f32 sun_angle = 0.0f;

            v3 sky_color{};
            f32 brightness = 0.0f;
        };

        std::vector<LightingData> sky_color_by_sun_angle
        {
            { 0.0f,   v3{ 255, 172, 77  } / 255.0f, 0.5f },
            { 20.0f,  v3{ 135, 207, 235 } / 255.0f, 1.0f },
            { 160.0f, v3{ 135, 207, 235 } / 255.0f, 1.0f },
            { 180.0f, v3{ 255, 172, 77  } / 255.0f, 0.5f },
            { 200.0f, v3{ 18,  17,  26  } / 255.0f, 0.1f },
            { 340.0f, v3{ 18,  17,  26  } / 255.0f, 0.1f },
        };

    private:

        i64 m_current_time = 0;

        EventHandle m_time_changed_handle{};

    };
}
