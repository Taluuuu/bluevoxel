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

        virtual void window(
            const std::string& title,
            const ui::Rect& rect,
            const std::function<void()>& window_contents) = 0;

        virtual void row(f32 height, i32 num_columns) = 0;

        virtual bool button(const std::string& title) = 0;

    };
}