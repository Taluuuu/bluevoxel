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

    void Gizmo::set_position(const v3& position)
    {
        m_position = position;
        m_grab_offset = v3{};
    }

    void Gizmo::update(f32 delta_time)
    {
        const auto& window = m_window_module->window();
        auto& renderer = m_rendering_module->renderer();

        const v3 mouse_ray_dir = h2o::physics::screen_to_ray_direction(
            m_input_module->mouse_position(),
            window.window_size(),
            renderer.view_matrix(),
            renderer.proj_matrix());

        const auto mouse_btn_state = m_input_module->mouse_button_state(h2o::MouseButton::Left);

        v3 hover_offset;
        v3i hovered_axes = find_hovered_axes(mouse_ray_dir, hover_offset);
        if (mouse_btn_state.held && !mouse_btn_state.pressed_this_frame)
            hovered_axes = {};

        draw_axis({ 1, 0, 0 }, hovered_axes.x, m_selected_axes.x);
        draw_axis({ 0, 1, 0 }, hovered_axes.y, m_selected_axes.y);
        draw_axis({ 0, 0, 1 }, hovered_axes.z, m_selected_axes.z);

        const bool is_any_axis_hovered = hovered_axes != v3i{};
        const bool is_any_axis_selected = m_selected_axes != v3i{};

        if (is_any_axis_selected)
        {
            if (mouse_btn_state.held)
            {
                // Move on selected axes
                const u32 num_selected_axes = m_selected_axes.x + m_selected_axes.y + m_selected_axes.z;
                switch (num_selected_axes)
                {
                case 1:
                {
                    const v3 axis{ m_selected_axes };

                    const auto& camera = renderer.camera();

                    // Find plane normal
                    m_position += m_grab_offset;
                    const v3 handled_location = m_position;
                    const v3 gizmo_to_cam = glm::normalize(camera.position() - handled_location);
                    const v3 temp = glm::cross(gizmo_to_cam, axis);
                    const v3 plane_normal = glm::normalize(glm::cross(axis, temp));

                    const h2o::physics::Plane plane{ m_position, plane_normal };
                    const h2o::physics::Ray ray{ camera.position(), mouse_ray_dir };

                    if (const auto t = h2o::physics::intersect_plane_two_directions(ray, plane))
                    {
                        const v3 previous_pos = m_position;
                        const v3 new_pos = ray.point_at(*t);

                        const v3 delta = new_pos - previous_pos;

                        m_position += glm::dot(delta, axis) * axis - m_grab_offset;

                        if (increment_size.has_value())
                            m_position = v3{ v3i{ m_position / *increment_size } } * *increment_size;
                    }

                    break;
                }

                case 2:
                case 3:
                    // TODO
                    break;

                default:
                    assert(false);
                    break;
                }
            }
            else
            {
                // Deselect axes
                m_selected_axes = {};
                m_input_module->clear_mouse_state(h2o::MouseCapturePriority::Editor);
            }
        }
        else if (is_any_axis_hovered)
        {
            if (mouse_btn_state.pressed_this_frame)
            {
                // Select axes
                m_selected_axes = hovered_axes;
                m_input_module->set_mouse_state(h2o::MouseCapturePriority::Editor, false);
                m_grab_offset = hover_offset;
            }
        }
    }

    void Gizmo::draw_axis(v3i axis, bool is_hovered, bool is_selected) const
    {
        const v3 handle_start = m_position;
        const v3 handle_end = handle_start + v3(axis) * handle_length;

        v4 handle_color;
        if (is_selected)
        {
            handle_color = held_color;
        }
        else if (is_hovered)
        {
            handle_color = hover_color;
        }
        else
        {
            handle_color = v4{ axis.x, axis.y, axis.z, 1.0f };
        }

        auto& renderer = m_rendering_module->renderer();
        renderer.draw_cylinder(handle_start, handle_end, handle_radius, handle_color);
    }

    v3i Gizmo::find_hovered_axes(const v3& mouse_ray_dir, v3& out_grab_offset) const
    {
        v3i result{};
        out_grab_offset = {};

        const auto& camera = m_rendering_module->renderer().camera();
        h2o::physics::Ray ray{ camera.position(), mouse_ray_dir };

        const v3 handle_start = m_position;

        const auto is_axis_hovered =
            [&](const v3i& axis) -> bool
            {
                const v3 handle_end = handle_start + v3(axis) * handle_length;
                const h2o::physics::Cylinder cylinder{ handle_start, handle_end, handle_radius };

                if (const auto t = h2o::physics::intersect_cylinder(ray, cylinder))
                {
                    out_grab_offset = ray.point_at(*t) - handle_start;
                    return true;
                }

                return false;
            };

        result += is_axis_hovered({ 1, 0, 0 }) ? v3i{ 1, 0, 0 } : v3i{};
        result += is_axis_hovered({ 0, 1, 0 }) ? v3i{ 0, 1, 0 } : v3i{};
        result += is_axis_hovered({ 0, 0, 1 }) ? v3i{ 0, 0, 1 } : v3i{};

        return result;
    }
}