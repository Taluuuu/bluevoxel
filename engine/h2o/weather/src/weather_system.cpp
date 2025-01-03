#include "weather/weather_system.h"

#include "core/engine.h"
#include "networking/net_peer.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "weather/weather_net_messages.h"

#include <glm/gtx/rotate_vector.hpp>

namespace h2o
{
    WeatherSystem::WeatherSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {
        if (const auto net_peer = scene.net_peer())
        {
            if (!net_peer->is_host())
            {
                net_peer->handle_message<net_msg::TimeChanged>(m_time_changed_handle,
                    [this](PeerID, const net_msg::TimeChanged& time_changed)
                    {
                        set_time(time_changed.new_time);
                    }
                );
            }
        }

        TickPhase::Type tick_phases = TickPhase::NetworkUpdate;
        if (scene.get_system<RenderingSystem>() != nullptr)
            tick_phases = tick_phases | TickPhase::Render;

        set_tick_phases(tick_phases);
    }

    void WeatherSystem::network_update(const f32 delta_time)
    {
        // TODO: Clearly I can come up with a better system for this...
        const auto net_peer = scene.net_peer();

        if (!net_peer || net_peer->is_host())
            set_time(m_current_time + 1);

        if (net_peer && net_peer->is_host())
        {
            const net_msg::TimeChanged time_changed_msg{ m_current_time };
            for (const auto peer : net_peer->peers())
                net_peer->send_message(peer, time_changed_msg);
        }
    }

    void WeatherSystem::render()
    {
        const auto rendering_system = scene.get_system<RenderingSystem>();
        if (!rendering_system)
            return;

        const auto camera = rendering_system->main_camera();
        if (!camera)
            return;

        auto& renderer = g_engine->get_module_checked<RenderingModule>().renderer();

        renderer.draw_sphere(
            camera->position() + m_sun_direction * sun_distance,
            sun_radius,
            v4{ current_lighting_settings().light_color, 1.0f }
        );

        renderer.set_clear_color(v4{ m_current_lighting_settings.sky_color, 1.0f });
    }

    void WeatherSystem::set_time(const i64 time)
    {
        m_current_time = time;

        const f32 sun_angle_degrees = fmodf(static_cast<f32>(m_current_time) * sun_speed, 360.0f);
        const f32 sun_angle_radians = glm::radians(sun_angle_degrees);
        m_sun_direction = v3{ glm::cos(sun_angle_radians), glm::sin(sun_angle_radians), 0.0f };
        m_sun_direction = glm::rotateY(m_sun_direction, glm::radians(30.0f));

        if (!lighting_settings.empty())
        {
            LightingSettings current_lighting_settings = lighting_settings[0];
            LightingSettings next_lighting_settings = lighting_settings[0];

            for (i32 i = 0; i < lighting_settings.size(); i++)
            {
                current_lighting_settings = next_lighting_settings;
                next_lighting_settings = lighting_settings[(i + 1) % lighting_settings.size()];

                if (current_lighting_settings.sun_angle <= sun_angle_degrees && sun_angle_degrees < next_lighting_settings.sun_angle)
                    break;
            }

            // Make sure the next color's angle is greater than the current's for interpolation
            if (next_lighting_settings.sun_angle < current_lighting_settings.sun_angle)
                next_lighting_settings.sun_angle += 360.0f;

            const f32 angular_range = next_lighting_settings.sun_angle - current_lighting_settings.sun_angle;
            const f32 angular_dist = sun_angle_degrees - current_lighting_settings.sun_angle;
            const f32 t = angular_dist / angular_range;

            m_current_lighting_settings = LightingSettings {
                .sun_angle = sun_angle_degrees,
                .sky_color = glm::mix(current_lighting_settings.sky_color, next_lighting_settings.sky_color, t),
                .light_color = glm::mix(current_lighting_settings.light_color, next_lighting_settings.light_color, t),
                .brightness = glm::mix(current_lighting_settings.brightness, next_lighting_settings.brightness, t)
            };
        }

        on_time_changed.broadcast(m_current_time);
    }
}
