#pragma once

#include "core/types.h"

#include <optional>

namespace h2o
{
    class Graph;

    struct GraphUIContext
    {
        std::optional<size_t> selected_point_index = std::nullopt;
        bool is_selected_point_grabbed = false;
    };
}

namespace h2o::graph
{
    void draw_ui(Graph& graph, GraphUIContext& ctx, bool flip_axes = false, v2 graph_size = v2{ 250.0f });
}
