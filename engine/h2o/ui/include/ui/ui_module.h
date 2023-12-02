#pragma once

#include "core/module.h"
#include "core/tickable.h"

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
    class UIModule
        : public Tickable
        , public IModule
    {
    public:

        // Tickable interface
        void frame_start(f32 delta_time) override;
        void frame_end(f32 delta_time) override;

        // IModule interface
        bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_ui"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    private:

        nk_glfw m_glfw { 0 };
        nk_context* m_nuklear_context{};

    };
}