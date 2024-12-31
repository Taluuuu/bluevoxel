#include "weather/weather_system.h"

#include <core/engine.h>

#include "../../rendering/include/rendering/camera.h"
#include "../../rendering/include/rendering/renderer.h"
#include "../../rendering/include/rendering/rendering_module.h"
#include "networking/net_peer.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "weather/weather_net_messages.h"

namespace h2o
{
    WeatherSystem::WeatherSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {
        if (const auto net_peer = m_scene->net_peer())
        {
            if (!net_peer->is_host())
            {
                net_peer->handle_message<net_msg::TimeChanged>(m_time_changed_handle,
                    [this](PeerID, const net_msg::TimeChanged& time_changed)
                    {
                        m_current_time = time_changed.new_time;
                    }
                );
            }
        }

        TickPhase::Type tick_phases = TickPhase::NetworkUpdate;
        if (m_scene->get_system<RenderingSystem>() != nullptr)
            tick_phases = tick_phases | TickPhase::Render;

        set_tick_phases(tick_phases);
    }

    void WeatherSystem::network_update(const f32 delta_time)
    {
        if (const auto net_peer = m_scene->net_peer())
        {
            if (net_peer->is_host())
            {
                m_current_time++;

                const net_msg::TimeChanged time_changed_msg{ m_current_time };
                for (const auto peer : net_peer->peers())
                    net_peer->send_message(peer, time_changed_msg);
            }
        }
    }

    void WeatherSystem::render()
    {
        const auto rendering_system = m_scene->get_system<RenderingSystem>();
        if (!rendering_system)
            return;

        const auto camera = rendering_system->main_camera();
        if (!camera)
            return;

        auto& renderer = g_engine->get_module_checked<RenderingModule>().renderer();

        const f32 sun_angle_degrees = fmodf(static_cast<f32>(m_current_time) * 1.0f, 360.0f);
        const f32 sun_angle_radians = glm::radians(sun_angle_degrees);
        const v3 sun_unit_pos{ glm::cos(sun_angle_radians), glm::sin(sun_angle_radians), 0.0f };

        renderer.draw_sphere(
            camera->position() + sun_unit_pos * sun_distance,
            sun_radius,
            sun_color
        );

        if (!sky_color_by_sun_angle.empty())
        {
            AngleColorPair current_color_pair = sky_color_by_sun_angle[0];
            AngleColorPair next_color_pair = sky_color_by_sun_angle[0];
            for (i32 i = 0; i < sky_color_by_sun_angle.size(); i++)
            {
                current_color_pair = next_color_pair;
                next_color_pair = sky_color_by_sun_angle[(i + 1) % sky_color_by_sun_angle.size()];

                if (current_color_pair.first <= sun_angle_degrees && sun_angle_degrees < next_color_pair.first)
                    break;
            }

            // Make sure the next color's angle is greater than the current's for interpolation
            if (next_color_pair.first < current_color_pair.first)
                next_color_pair.first += 360.0f;

            const f32 angular_range = next_color_pair.first - current_color_pair.first;
            const f32 angular_dist = sun_angle_degrees - current_color_pair.first;

            const v3 final_sky_color = glm::mix(current_color_pair.second, next_color_pair.second, angular_dist / angular_range);

            renderer.set_clear_color(v4{ final_sky_color, 1.0f });
        }
    }
}
