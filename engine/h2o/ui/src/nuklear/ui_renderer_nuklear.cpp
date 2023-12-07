#include "ui_renderer_nuklear.h"

#include "core/engine.h"
#include "nuklear_utils.h"
#include "windowing/windowing_module.h"

namespace h2o
{
    UIRenderer_Nuklear::UIRenderer_Nuklear(Tickable* owner)
        : Tickable(owner)
    {}

    bool UIRenderer_Nuklear::init(Engine& engine)
    {
        const auto& windowing_module = engine.get_module_checked<WindowingModule>();
        const IWindow& window = windowing_module.window();

        if (m_nk_context = nk_glfw3_init(&m_glfw, static_cast<GLFWwindow*>(window.handle()), nk_glfw_init_state::NK_GLFW3_INSTALL_CALLBACKS); !m_nk_context)
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
        return true;
    }

    void UIRenderer_Nuklear::cleanup()
    {
        nk_glfw3_shutdown(&m_glfw);
    }

    void UIRenderer_Nuklear::window(
        const std::string& title,
        const ui::Rect& rect,
        const std::function<void()>& window_contents)
    {
        nk_begin(m_nk_context, title.c_str(), ui::to_nk_rect(rect), 0);

        window_contents();

        nk_end(m_nk_context);
    }

    void UIRenderer_Nuklear::row(f32 height, i32 num_columns)
    {
        nk_layout_row_dynamic(m_nk_context, height, num_columns);
    }

    bool UIRenderer_Nuklear::button(const std::string& title)
    {
        return nk_button_label(m_nk_context, title.c_str());
    }

    void UIRenderer_Nuklear::frame_start(f32 delta_time)
    {
        nk_glfw3_new_frame(&m_glfw);
    }

    void UIRenderer_Nuklear::frame_end(f32 delta_time)
    {
        nk_glfw3_render(&m_glfw, NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
    }
}