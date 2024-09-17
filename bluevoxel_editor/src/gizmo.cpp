#include "gizmo.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "physics/ray_intersections.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "selection_manager.h"
#include "windowing/windowing_module.h"

namespace bluevoxel
{
    Gizmo::Gizmo(h2o::Tickable* tickable, SelectionManager& selection_manager)
        : Tickable(tickable)
        , m_selection_mgr(&selection_manager)
        , m_input_module(&g_engine->get_module_checked<h2o::InputModule>())
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
    {
        set_tick_phases(h2o::TickPhase::Update);
    }

    void Gizmo::set_position(const v3& position)
    {
        m_position = align_to_grid(position);
        m_movement_delta = v3{};
        m_grab_offset = v3{};
    }

    void Gizmo::update(f32 delta_time)
    {
        auto& renderer = m_rendering_module->renderer();

        const auto& camera = renderer.camera();
        const v3 camera_pos = camera.position();
        const v3 camera_front = camera.front();

        const f32 distance_with_camera = glm::length(camera_pos - m_position);
        const f32 handle_radius_ = handle_radius * distance_with_camera;

        const auto add_to_selection_manager =
            [&](const v3i& axis)
            {
                const v3 handle_end = m_position + v3(handle_direction(axis, camera_front)) * handle_length * distance_with_camera;

                const phys::Cylinder cylinder{ m_position, handle_end, handle_radius_ };
                m_selection_mgr->add(cylinder,
                    [this, axis](const HoverData& hover_data)
                    {
                        m_hovered_axes = {};

                        if (hover_data.click_state.pressed_this_frame)
                        {
                            // Start moving the selected axis
                            m_selected_axes = axis;
                            m_selection_mgr->pause_selection();
                            g_engine->layer_stack().push_layer(h2o::Layer::UI, h2o::LayerData{ false, false });
                            m_grab_offset = m_selection_mgr->mouse_ray().point_at(hover_data.t) - m_position;
                        }
                        else
                        {
                            m_hovered_axes = axis;
                        }
                    }
                );
            };

        add_to_selection_manager({ 1, 0, 0 });
        add_to_selection_manager({ 0, 1, 0 });
        add_to_selection_manager({ 0, 0, 1 });

        draw_axis({ 1, 0, 0 }, m_hovered_axes.x, m_selected_axes.x);
        draw_axis({ 0, 1, 0 }, m_hovered_axes.y, m_selected_axes.y);
        draw_axis({ 0, 0, 1 }, m_hovered_axes.z, m_selected_axes.z);

        const bool is_any_axis_selected = m_selected_axes != v3i{};

        const auto mouse_btn_state = m_input_module->mouse_button_state(h2o::MouseButton::Left);
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

                    const v3 prev_position = m_position;
                    m_position += m_grab_offset;

                    // Find plane normal
                    const v3 handled_location = m_position;
                    const v3 gizmo_to_cam = glm::normalize(camera.position() - handled_location);
                    const v3 temp = glm::cross(gizmo_to_cam, axis);
                    const v3 plane_normal = glm::normalize(glm::cross(axis, temp));

                    const h2o::physics::Plane plane{ m_position, -plane_normal };
                    const auto& ray = m_selection_mgr->mouse_ray();

                    if (const auto t = h2o::physics::intersect(ray, plane))
                    {
                        const v3 previous_pos = m_position;
                        const v3 new_pos = ray.point_at(*t);

                        const v3 delta = new_pos - previous_pos;

                        m_position += glm::dot(delta, axis) * axis - m_grab_offset;
                        m_position = align_to_grid(m_position);

                        m_movement_delta = m_position - prev_position;
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
                m_selection_mgr->resume_selection();
                g_engine->layer_stack().pop_layer(h2o::Layer::UI);
            }
        }

        m_hovered_axes = v3i{};
    }

    void Gizmo::draw_axis(v3i axis, bool is_hovered, bool is_selected) const
    {
        auto& renderer = m_rendering_module->renderer();
        const auto& camera = renderer.camera();

        const f32 distance_with_camera = glm::length(camera.position() - m_position);
        const v3i handle_dir = handle_direction(axis, camera.front());

        const v3 handle_start = m_position;
        const v3 handle_end = handle_start + v3{ handle_dir } * handle_length * distance_with_camera;
        const f32 handle_radius_ = handle_radius * distance_with_camera;

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

        renderer.draw_cylinder(handle_start, handle_end, handle_radius_, handle_color);
    }

    v3 Gizmo::align_to_grid(const v3& position) const
    {
        const v3 clamped_position = bounds ?
            glm::clamp(position, bounds->min, bounds->max) :
            position;

        if (increment_size)
            return v3{ v3i{ clamped_position / *increment_size } } * *increment_size;

        return clamped_position;
    }

    v3i Gizmo::handle_direction(const v3i& axis, const v3& camera_front) const
    {
        return glm::dot(camera_front, v3{ axis }) < 0.0f ? axis : -axis;
    }
}