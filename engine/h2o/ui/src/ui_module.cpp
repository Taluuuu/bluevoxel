#include "ui/ui_module.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "rendering/rendering_module.h"
//#include "nuklear/ui_renderer_nuklear.h"
#include "windowing/windowing_module.h"

#include <imgui.h>

namespace h2o
{
    UIModule::UIModule()
        : Tickable(g_engine)
    {}

    void UIModule::window(
        const std::string& title,
        const ui::Rect& rect,
        const std::function<void(IUIRenderer&)>& window_contents)
    {
//        assert(m_ui_renderer);
//
//        if (m_ui_renderer->window_begin(title, rect))
//            window_contents(*m_ui_renderer);
//
//        m_ui_renderer->window_end();
    }

    bool UIModule::init(Engine& engine)
    {
        m_input_module = &engine.get_module_checked<InputModule>();

//        m_ui_renderer = std::make_shared<UIRenderer_Nuklear>();
//        if (!m_ui_renderer->init(engine))
//            return false;
//
        set_tick_phases(TickPhase::FrameStart | TickPhase::FrameEnd);

        return true;
    }

    void UIModule::cleanup()
    {
//        m_ui_renderer->cleanup();
//        m_ui_renderer.reset();
    }

    std::vector<std::type_index> UIModule::dependencies() const
    {
        return {
            typeid(RenderingModule),
            typeid(WindowingModule),
            typeid(InputModule) };
    }

    void UIModule::frame_start(f32 delta_time)
    {
//        assert(m_ui_renderer);
//        m_ui_renderer->frame_start();
    }

    void UIModule::frame_end(f32 delta_time)
    {
//        assert(m_ui_renderer);
//        m_ui_renderer->frame_end();

        if (ImGui::GetIO().WantCaptureMouse && !m_input_module->is_mouse_captured())
        {
            m_input_module->set_mouse_state(h2o::MouseCapturePriority::UI, false);
        }
        else
        {
            m_input_module->clear_mouse_state(h2o::MouseCapturePriority::UI);
        }
    }
}