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

        void update(f32 delta_time) override;

    protected:

        enum class Axis { X, Y, Z };
        [[nodiscard]] static constexpr v3i to_direction(Axis axis)
        {
            switch (axis)
            {
            case Axis::X: return { 1, 0, 0 };
            case Axis::Y: return { 0, 1, 0 };
            case Axis::Z: return { 0, 0, 1 };
            }
        }

        void draw_axis(Axis axis, const v3& mouse_ray_dir) const;

    protected:

        static constexpr f32 handle_length = 2.0f;
        static constexpr f32 handle_radius = 0.05f;

        static constexpr v4 hover_color{ 1.0f, 1.0f, 0.0f, 1.0f };
        static constexpr v4 held_color{ 1.0f, 1.0f, 1.0f, 1.0f };

    private:

        // Module refs
        h2o::InputModule* const m_input_module = nullptr;
        h2o::RenderingModule* const m_rendering_module = nullptr;
        h2o::WindowingModule* const m_window_module = nullptr;

    };
}