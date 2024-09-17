#pragma once

#include "core/types.h"

#include <optional>
#include <vector>

namespace h2o
{
    enum class Layer : i32
    {
        Game = 0,  // Camera controls and the like
        UI,        // Inventory screens, dialogs, ...
        PauseMenu, // Pause menu

        Last = PauseMenu,
        None
    };

    struct LayerData
    {
        bool capture_mouse = false;
        bool allow_ui_interaction = true;
    };

    class LayerStack
    {
    public:

        LayerStack();

        void push_layer(Layer layer, const LayerData& layer_data);
        void pop_layer(Layer layer);

        [[nodiscard]] const LayerData& top_layer_data() const;
        [[nodiscard]] Layer top_layer() const;

    private:

        std::vector< std::optional<LayerData> > m_layers{};

    };
}
