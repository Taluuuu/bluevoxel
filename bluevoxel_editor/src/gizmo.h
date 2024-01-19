#pragma once

#include "core/tickable.h"

namespace h2o
{
    class InputModule;
    class RenderingModule;
    class WindowingModule;
}

namespace bluevoxel
{
    class Gizmo : public h2o::Tickable
    {
    public:

        explicit Gizmo(h2o::Tickable* tickable);
        ~Gizmo() override = default;

        [[nodiscard]] const v3& position() const { return m_position; }
        void set_position(const v3& position) { m_position = position; }

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;

        void draw_axis(v3i axis, bool is_hovered, bool is_selected) const;
        [[nodiscard]] v3i find_hovered_axes(const v3& mouse_ray_dir, v3& out_grab_offset) const;

    protected:

        static constexpr f32 handle_length = 2.0f;
        static constexpr f32 handle_radius = 0.05f;

        static constexpr v4 hover_color{ 1.0f, 1.0f, 0.0f, 1.0f };
        static constexpr v4 held_color{ 1.0f, 1.0f, 1.0f, 1.0f };

    private:

        v3 m_position{};
        v3 m_grab_offset{};
        v3i m_selected_axes{};

        // Module refs
        h2o::InputModule* const m_input_module = nullptr;
        h2o::RenderingModule* const m_rendering_module = nullptr;
        h2o::WindowingModule* const m_window_module = nullptr;

    };
}