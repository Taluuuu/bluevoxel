#include "ui/ui_module.h"

#include "core/engine.h"
#include "rendering/rendering_module.h"
#include "windowing/windowing_module.h"

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
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include <nuklear.h>
#include <demo/glfw_opengl3/nuklear_glfw_gl3.h>

namespace h2o
{
    bool UIModule::init(Engine& engine)
    {
        const auto& windowing_module = engine.get_module_checked<WindowingModule>();
        const IWindow& window = windowing_module.window();

        if (m_nuklear_context = nk_glfw3_init(&m_glfw, static_cast<GLFWwindow*>(window.handle()), nk_glfw_init_state::NK_GLFW3_INSTALL_CALLBACKS); !m_nuklear_context)
        {
            log::error("Failed to initialize Nuklear.");
            return false;
        }

        {
            struct nk_font_atlas* atlas;
            nk_glfw3_font_stash_begin(&m_glfw, &atlas);
            // Load fonts here in the future
            nk_glfw3_font_stash_end(&m_glfw);
        }

        set_tick_phases(TickPhase::FrameStart | TickPhase::FrameEnd);
    }

    void UIModule::cleanup()
    {

    }

    std::vector<std::type_index> UIModule::dependencies() const
    {
        return { typeid(RenderingModule), typeid(WindowingModule) };
    }

    void UIModule::frame_start(f32 delta_time)
    {
        nk_glfw3_new_frame(&m_glfw);

        if (nk_begin(m_nuklear_context, "Demo", nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;
            nk_layout_row_static(m_nuklear_context, 30, 80, 1);
            if (nk_button_label(m_nuklear_context, "button"))
                fprintf(stdout, "button pressed\n");

            nk_layout_row_dynamic(m_nuklear_context, 30, 2);
            if (nk_option_label(m_nuklear_context, "easy", op == EASY)) op = EASY;
            if (nk_option_label(m_nuklear_context, "hard", op == HARD)) op = HARD;

            nk_layout_row_dynamic(m_nuklear_context, 25, 1);
            nk_property_int(m_nuklear_context, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(m_nuklear_context, 20, 1);
            nk_label(m_nuklear_context, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(m_nuklear_context, 25, 1);
            if (nk_combo_begin_color(m_nuklear_context, nk_rgb_cf(nk_colorf{}), nk_vec2(nk_widget_width(m_nuklear_context),400))) {
                nk_layout_row_dynamic(m_nuklear_context, 120, 1);
                nk_color_picker(m_nuklear_context, nk_colorf{}, NK_RGBA);
                nk_layout_row_dynamic(m_nuklear_context, 25, 1);
                nk_propertyf(m_nuklear_context, "#R:", 0, 0.0f, 1.0f, 0.01f,0.005f);
                nk_propertyf(m_nuklear_context, "#G:", 0, 0.0f, 1.0f, 0.01f,0.005f);
                nk_propertyf(m_nuklear_context, "#B:", 0, 0.0f, 1.0f, 0.01f,0.005f);
                nk_propertyf(m_nuklear_context, "#A:", 0, 0.0f, 1.0f, 0.01f,0.005f);
                nk_combo_end(m_nuklear_context);
            }
        }
        nk_end(m_nuklear_context);
    }

    void UIModule::frame_end(f32 delta_time)
    {
        nk_glfw3_render(&m_glfw, NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
    }
}