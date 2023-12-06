#pragma once

#include "core/tickable.h"
#include "ui/ui_renderer.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear.h>
#include <demo/glfw_opengl3/nuklear_glfw_gl3.h>

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

        // Tickable interface
        void frame_start(f32 delta_time) override;
        void frame_end(f32 delta_time) override;

    private:

        nk_glfw m_glfw { nullptr };
        nk_context* m_nk_context = nullptr;

    };
}