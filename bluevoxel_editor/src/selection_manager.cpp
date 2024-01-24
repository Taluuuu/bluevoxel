#include "selection_manager.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "physics/math_helpers.h"
#include "physics/ray.h"
#include "physics/ray_intersections.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "windowing/windowing_module.h"

namespace bluevoxel
{
    SelectionManager::SelectionManager(h2o::Tickable* owner)
        : Tickable(owner)
        , m_input_module(&g_engine->get_module_checked<h2o::InputModule>())
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_window_module(&g_engine->get_module_checked<h2o::WindowingModule>())
    {
        set_tick_phases(h2o::TickPhase::FrameStart | h2o::TickPhase::PostUpdate);
    }

    void SelectionManager::pause_selection()
    {
        m_is_selection_enabled = false;
    }

    void SelectionManager::resume_selection()
    {
        m_is_selection_enabled = true;
    }

    template<>
    void SelectionManager::add(const phys::Cylinder& primitive, const SelectionCallback& callback)
    {
        if (m_is_selection_enabled)
            m_cylinders.emplace_back(primitive, callback);
    }

    template<>
    void SelectionManager::add(const phys::Triangle& primitive, const SelectionCallback& callback)
    {
        if (m_is_selection_enabled)
            m_triangles.emplace_back(primitive, callback);
    }

    template<>
    void SelectionManager::add(const phys::Sphere& primitive, const SelectionCallback& callback)
    {
        if (m_is_selection_enabled)
            m_spheres.emplace_back(primitive, callback);
    }

    static void intersect_primitives(
        const phys::Ray& ray, auto& primitives, f32& min_t,
        SelectionCallback& out_closest_primitive_callback)
    {
        for (const auto& [primitive, callback] : primitives)
        {
            if (const auto t = phys::intersect(ray, primitive))
            {
                if (min_t > *t)
                {
                    min_t = *t;
                    out_closest_primitive_callback = callback;
                }
            }
        }

        primitives.clear();
    }

    void SelectionManager::frame_start(f32 delta_time)
    {
        const auto& window = m_window_module->window();
        const auto& renderer = m_rendering_module->renderer();
        const auto& camera = renderer.camera();

        const v3 mouse_ray_dir = h2o::physics::screen_to_ray_direction(
            m_input_module->mouse_position(),
            window.window_size(),
            renderer.view_matrix(),
            renderer.proj_matrix());

        m_mouse_ray = { camera.position(), mouse_ray_dir };
    }

    void SelectionManager::post_update(f32 delta_time)
    {
        f32 min_t = FLT_MAX;
        SelectionCallback closest_primitive_callback{};

        intersect_primitives(m_mouse_ray, m_cylinders, min_t, closest_primitive_callback);
        intersect_primitives(m_mouse_ray, m_spheres,   min_t, closest_primitive_callback);
        intersect_primitives(m_mouse_ray, m_triangles, min_t, closest_primitive_callback);

        // Only call callback if the mouse is not captured or the mouse was just clicked this frame
        const auto click_state = m_input_module->mouse_button_state(h2o::MouseButton::Left);
        if (!m_input_module->is_mouse_captured() || click_state.pressed_this_frame)
        {
            if (closest_primitive_callback)
                closest_primitive_callback({ click_state, min_t });
        }
    }
}