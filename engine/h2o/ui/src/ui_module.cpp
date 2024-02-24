#include "ui/ui_module.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "rendering/rendering_module.h"
#include "windowing/windowing_module.h"

#include "imgui.h"

namespace h2o
{
    UIModule::UIModule()
        : Tickable(g_engine)
    {}

    bool UIModule::is_interacting_with_ui()
    {
        return ImGui::GetIO().WantCaptureMouse;
    }

    bool UIModule::init(Engine& engine)
    {
        m_input_module = &engine.get_module_checked<InputModule>();

        set_tick_phases(TickPhase::FrameEnd);

        return true;
    }

    std::vector<std::type_index> UIModule::dependencies() const
    {
        return {
            typeid(RenderingModule),
            typeid(WindowingModule),
            typeid(InputModule) };
    }

    void UIModule::frame_end(f32 delta_time)
    {
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