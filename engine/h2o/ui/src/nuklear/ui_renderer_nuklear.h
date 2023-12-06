#pragma once

#include "core/tickable.h"
#include "ui/ui_renderer.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "nuklear_headers.h"

namespace h2o
{
    class UIRenderer_Nuklear
        : public Tickable
        , public IUIRenderer
    {
    public:

        explicit UIRenderer_Nuklear(Tickable* owner);
        ~UIRenderer_Nuklear() override = default;

        // IRendererUI interface
        bool init(Engine& engine) override;
        void cleanup() override;
        void window(const std::string& title, const ui::Rect& rect, const std::function<void()>& window_contents) override;
        void row(f32 height, i32 num_columns) override;
        bool button(const std::string& title) override;

        // Tickable interface
        void frame_start(f32 delta_time) override;
        void frame_end(f32 delta_time) override;

    private:

        nk_glfw m_glfw { nullptr };
        nk_context* m_nk_context = nullptr;

    };
}