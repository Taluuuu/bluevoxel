#include "ui/ui_module.h"

#include "core/engine.h"
#include "core/layers.h"
#include "imgui.h"
#include "input/input_module.h"
#include "rendering/rendering_module.h"
#include "windowing/windowing_module.h"

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
        auto& layer_stack = g_engine->layer_stack();
        if (layer_stack.top_layer() == Layer::PauseMenu || m_keep_stats_open)
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

                        if (const std::string* val = std::get_if<std::string>(&value))
                        {
                            ImGui::Text("%s: %s", name.c_str(), val->c_str());
                            continue;
                        }
                    }
                }

                ImGui::Checkbox("Keep Open", &m_keep_stats_open);
            }
            ImGui::End();
        }
    }

    void UIModule::frame_end(f32 delta_time)
    {
        auto& layer_stack = g_engine->layer_stack();
        const auto& layer_data = layer_stack.top_layer_data();

        auto& io = ImGui::GetIO();
        if (io.WantCaptureMouse && layer_data.allow_ui_interaction)
        {
            layer_stack.push_layer(Layer::UI, LayerData{ false, true });
        }
        else
        {
            layer_stack.pop_layer(Layer::UI);
        }

        if (layer_data.allow_ui_interaction)
        {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }
        else
        {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        }
    }
}