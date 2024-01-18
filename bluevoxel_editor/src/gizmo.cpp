#include "gizmo.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "physics/math_helpers.h"
#include "physics/ray_intersections.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "windowing/windowing_module.h"

namespace bluevoxel
{
    Gizmo::Gizmo(h2o::Tickable* tickable)
        : Tickable(tickable)
        , m_input_module(&g_engine->get_module_checked<h2o::InputModule>())
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_window_module(&g_engine->get_module_checked<h2o::WindowingModule>())
    {
        set_tick_phases(h2o::TickPhase::Update);
    }

    void Gizmo::update(f32 delta_time)
    {
        const auto& window = m_window_module->window();
        const auto& renderer = m_rendering_module->renderer();

        const v3 mouse_ray_dir = h2o::physics::screen_to_ray_direction(
            m_input_module->mouse_position(),
            window.window_size(),
            renderer.view_matrix(),
            renderer.proj_matrix());

        draw_axis(Axis::X, mouse_ray_dir);
        draw_axis(Axis::Y, mouse_ray_dir);
        draw_axis(Axis::Z, mouse_ray_dir);
    }

    void Gizmo::draw_axis(Gizmo::Axis axis, const v3& mouse_ray_dir) const
    {
        auto& renderer = m_rendering_module->renderer();
        const auto& camera = renderer.camera();

        const v3i axis_dir = to_direction(axis);

        const v3 handle_start{ 0.0f, 0.0f, 0.0f };
        const v3 handle_end = handle_start + v3(axis_dir) * handle_length;

        const auto t = h2o::physics::intersect_cylinder(
            h2o::physics::Ray{ camera.position(), mouse_ray_dir },
            h2o::physics::Cylinder{ handle_start, handle_end, handle_radius });

        const v4 handle_color = t.has_value() ?
            hover_color :
            v4{ axis_dir.x, axis_dir.y, axis_dir.z, 1.0f };

        renderer.draw_cylinder(handle_start, handle_end, handle_radius, handle_color);
    }
}