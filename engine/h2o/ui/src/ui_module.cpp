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

        set_tick_phases(TickPhase::PostUpdate | TickPhase::FrameEnd);

        return true;
    }

    std::vector<std::type_index> UIModule::dependencies() const
    {
        return {
            typeid(RenderingModule),
            typeid(WindowingModule),
            typeid(InputModule) };
    }

    void UIModule::post_update(f32 delta_time)
    {
        if (ImGui::Begin("Stats"))
        {
            const auto& categories = g_engine->debug_infos().debug_stats_by_category();
            for (const auto& [category, stats] : categories)
            {
                if (!ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    continue;

                for (const auto& [name, value] : stats)
                {
                    if (const i32* val = std::get_if<i32>(&value))
                    {
                        ImGui::Text("%s: %i", name.c_str(), *val);
                        continue;
                    }

                    if (const f32* val = std::get_if<f32>(&value))
                    {
                        ImGui::Text("%s: %f", name.c_str(), *val);
                        continue;
                    }
                }
            }
        }
        ImGui::End();
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