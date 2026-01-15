#include "graph/graph_ui.h"

#include "core/log.h"
#include "graph/graph.h"
#include "ui/imgui.h"

namespace h2o
{
    void graph::draw_ui(Graph& graph, GraphUIContext& ctx, const bool flip_axes, const v2 graph_size)
    {
        auto* draw_list = ImGui::GetWindowDrawList();
        if (!draw_list)
            return;

        const v2 cursor_pos = ImGui::GetCursorScreenPos();
        const v2 mouse_pos = ImGui::GetMousePos();

        draw_list->AddRectFilled(
            cursor_pos,
            cursor_pos + graph_size,
            ImColor(0.2f, 0.2f, 0.2f, 1.0f));

        const auto map_val =
            [&](const f32 val, const f32 a_min, const f32 a_max, const f32 b_min, const f32 b_max)
            {
                return ((val - a_min) / (a_max - a_min)) * (b_max - b_min) + b_min;
            };

        const auto map_vec =
            [&](const v2 val, const v2 a_min, const v2 a_max, const v2 b_min, const v2 b_max)
            {
                return v2{
                    map_val(val.x, a_min.x, a_max.x, b_min.x, b_max.x),
                    map_val(val.y, a_min.y, a_max.y, b_max.y, b_min.y)};
            };

        const auto to_screen_pos = [&](const v2 graph_pos)
        {
            if (flip_axes)
            {
                return v2{
                    map_val(graph_pos.y, graph.graph_min().y, graph.graph_max().y, cursor_pos.x, cursor_pos.x + graph_size.x),
                    map_val(graph_pos.x, graph.graph_min().x, graph.graph_max().x, cursor_pos.y + graph_size.y, cursor_pos.y)
                };
            }

            return map_vec(graph_pos, graph.graph_min(), graph.graph_max(), cursor_pos, cursor_pos + graph_size);
        };

        const auto to_graph_pos = [&](const v2 screen_pos)
        {
            if (flip_axes)
            {
                return v2{
                    map_val(screen_pos.y, cursor_pos.y + graph_size.y, cursor_pos.y, graph.graph_min().x, graph.graph_max().x),
                    map_val(screen_pos.x, cursor_pos.x, cursor_pos.x + graph_size.x, graph.graph_min().y, graph.graph_max().y)
                };
            }

            return map_vec(screen_pos, cursor_pos, cursor_pos + graph_size, graph.graph_min(), graph.graph_max());
        };

        if (ctx.selected_point_index && ctx.is_selected_point_grabbed)
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                // Move grabbed point
                graph.edit_point(*ctx.selected_point_index, to_graph_pos(mouse_pos));
            }
            else
            {
                ctx.is_selected_point_grabbed = false;
            }
        }

        // TODO: Custom name to avoid collision
        ImGui::InvisibleButton("Graph", graph_size);

        {
            const v2 screen_min = to_screen_pos(graph.graph_min());
            const v2 screen_max = to_screen_pos(graph.graph_max());
            const v2 screen_center = to_screen_pos(v2{0.0f});
            draw_list->AddLine(v2{screen_min.x, screen_center.y}, v2{screen_max.x, screen_center.y}, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
            draw_list->AddLine(v2{screen_center.x, screen_min.y}, v2{screen_center.x, screen_max.y}, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
        }

        for (size_t i = 0; i < graph.num_points(); i++)
        {
            constexpr f32 point_radius = 6.0f;

            const v2 point_on_screen = to_screen_pos(graph.get_value_by_index(i).value_or(v2{}));
            draw_list->AddCircleFilled(point_on_screen, point_radius,
                (ctx.selected_point_index == i) ?
                    ImColor(1.0f, 1.0f, 0.0f, 1.0f) :
                    ImColor(1.0f, 1.0f, 1.0f, 1.0f));

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ctx.is_selected_point_grabbed)
            {
                if (glm::distance(mouse_pos, point_on_screen) < point_radius)
                {
                    ctx.selected_point_index = i;
                    ctx.is_selected_point_grabbed = true;
                }
                else
                {
                    ctx.selected_point_index = std::nullopt;
                    ctx.is_selected_point_grabbed = false;
                }
            }

            if (auto next_point = graph.get_value_by_index(i + 1))
            {
                const v2 next_point_on_screen = to_screen_pos(*next_point);
                draw_list->AddLine(point_on_screen, next_point_on_screen,
                    ImColor(0.0f, 1.0f, 1.0f, 1.0f));
            }
        }

        const v2 button_size{ graph_size.x / 2.0f - ImGui::GetStyle().FramePadding.x, 30.0f };
        if (ImGui::Button("Add Point", button_size))
            graph.add_point(glm::mix(graph.graph_min(), graph.graph_max(), 0.5f));

        ImGui::SameLine();

        if (ImGui::Button("Remove Point", button_size) && ctx.selected_point_index)
            graph.remove_point(*ctx.selected_point_index);
    }
}
