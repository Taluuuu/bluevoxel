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
    class SelectionManager;

    class Gizmo : public h2o::Tickable
    {
    public:

        Gizmo(h2o::Tickable* tickable, SelectionManager& selection_manager);
        ~Gizmo() override = default;

        [[nodiscard]] const v3& movement_delta() const { return m_movement_delta; };
        [[nodiscard]] const v3& position() const { return m_position; }
        void set_position(const v3& position);

    public:

        std::optional<f32> increment_size = std::nullopt;

        struct Bounds{ v3 min{}, max{}; };
        std::optional<Bounds> bounds = std::nullopt;

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;

        void draw_axis(v3i axis, bool is_hovered, bool is_selected) const;

        [[nodiscard]] v3 align_to_grid(const v3& position) const;
        [[nodiscard]] v3i handle_direction(const v3i& axis, const v3& camera_front) const;

    protected:

        static constexpr f32 handle_length = 0.25f;
        static constexpr f32 handle_radius = 0.01f;

        static constexpr v4 hover_color{ 1.0f, 1.0f, 0.0f, 1.0f };
        static constexpr v4 held_color{ 1.0f, 1.0f, 1.0f, 1.0f };

    private:

        v3 m_position{};
        v3 m_movement_delta{};
        v3 m_grab_offset{};
        v3i m_hovered_axes{};
        v3i m_selected_axes{};

        SelectionManager* const m_selection_mgr = nullptr;

        // Module refs
        h2o::InputModule* const m_input_module = nullptr;
        h2o::RenderingModule* const m_rendering_module = nullptr;

    };
}