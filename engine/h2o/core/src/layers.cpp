#include "core/layers.h"

namespace h2o
{
    LayerStack::LayerStack()
    {
        m_layers.assign(static_cast<i32>(Layer::Last), std::nullopt);
    }

    void LayerStack::push_layer(Layer layer, const LayerData& layer_data)
    {
        m_layers[static_cast<i32>(layer)] = layer_data;
    }

    void LayerStack::pop_layer(Layer layer)
    {
        m_layers[static_cast<i32>(layer)] = std::nullopt;
    }

    const LayerData& LayerStack::top_layer_data() const
    {
        for (i32 i = static_cast<i32>(Layer::Last); i >= 0; --i)
        {
            if (const auto& layer = m_layers[i]; layer.has_value())
                return *layer;
        }

        static LayerData empty_layer_data{};
        return empty_layer_data;
    }

    Layer LayerStack::top_layer() const
    {
        for (i32 i = static_cast<i32>(Layer::Last); i >= 0; --i)
        {
            if (const auto& layer = m_layers[i]; layer.has_value())
                return static_cast<Layer>(i);
        }

        return Layer::None;
    }
}
