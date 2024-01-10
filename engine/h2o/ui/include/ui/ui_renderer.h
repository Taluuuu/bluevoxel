#pragma once

#include <functional>

#include "ui_types.h"

namespace h2o
{
    class Engine;

    class IUIRenderer
    {
    public:

        virtual ~IUIRenderer() = default;

        virtual bool init(Engine& engine) = 0;
        virtual void cleanup() = 0;

        virtual void frame_start() = 0;
        virtual void frame_end() = 0;

        [[nodiscard]] virtual bool is_mouse_over_ui() const = 0;
        [[nodiscard]] virtual bool is_mouse_over_next_widget() const = 0;

        virtual void row(f32 height, i32 num_columns) = 0;

        virtual void label(const std::string& label) = 0;

        virtual bool button(const std::string& title) = 0;

        virtual bool input_text(const std::string& label, std::string& text) = 0;
        virtual bool input_int(const std::string& label, i32& num) = 0;

        virtual bool combobox(const std::vector<const char*>& options, u32& selected_index) = 0;

        // Not meant to be called directly by the user
        virtual bool window_begin(
            const std::string& title,
            const ui::Rect& rect) = 0;
        virtual void window_end() = 0;

    };
}