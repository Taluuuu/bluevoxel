#pragma once

#include "core/events.h"
#include "scene/scene_system.h"

namespace h2o
{
    class WeatherSystem : public SceneSystem
    {
    public:

        explicit WeatherSystem(const SceneSystemInitializer& system_initializer);

        struct LightingSettings
        {
            f32 sun_angle = 0.0f;
            v3 sky_color{};
            v3 light_color{};
            f32 brightness = 0.0f;
        };

        [[nodiscard]] const LightingSettings& current_lighting_settings() const { return m_current_lighting_settings; }
        [[nodiscard]] v3 sun_direction() const { return m_sun_direction; }

        Event<i64> on_time_changed{};

        // Tickable interface
        void network_update(f32 delta_time) override;
        void render() override;

    public:

        f32 sun_distance = 300.0f;
        f32 sun_radius = 30.0f;
        f32 sun_speed = 1.0f;//0.01f;
        f32 sun_angle_y = 30.0f; // in degrees

        f32 moon_distance = 300.0f;
        f32 moon_radius = 20.0f;
        v3 moon_color = v3{ 175, 175, 175 } / 255.0f;

        std::vector<LightingSettings> lighting_settings
        {
            { 0.0f,   v3{ 255, 172, 77  } / 255.0f, v3{ 252, 214, 172 } / 255.0f, 0.5f },
            { 20.0f,  v3{ 135, 207, 235 } / 255.0f, v3{ 250, 242, 209 } / 255.0f, 1.0f },
            { 160.0f, v3{ 135, 207, 235 } / 255.0f, v3{ 250, 242, 209 } / 255.0f, 1.0f },
            { 180.0f, v3{ 255, 172, 77  } / 255.0f, v3{ 252, 214, 172 } / 255.0f, 0.5f },
            { 200.0f, v3{ 18,  17,  26  } / 255.0f, v3{ 130, 152, 209 } / 255.0f, 0.3f },
            { 340.0f, v3{ 18,  17,  26  } / 255.0f, v3{ 130, 152, 209 } / 255.0f, 0.3f },
        };

    protected:

        void set_time(i64 time);

    private:

        i64 m_current_time = 0;

        LightingSettings m_current_lighting_settings{
            90.0f,
            v3{ 135, 207, 235 } / 255.0f,
            v3{ 250, 242, 209 } / 255.0f,
            1.0f
        };

        v3 m_sun_direction{ 0.0f, -1.0f, 0.0f };

        EventHandle m_time_changed_handle{};

    };
}
