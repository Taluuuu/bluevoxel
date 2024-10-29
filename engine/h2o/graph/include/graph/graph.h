#pragma once

#include "core/types.h"
#include "core/yaml.h"

#include <optional>
#include <vector>

namespace h2o
{
    class Graph
    {
    public:

        explicit Graph(v2 graph_min = v2{0.0f}, v2 graph_max = v2{1.0f});

        [[nodiscard]] size_t num_points() const { return m_points.size(); }
        [[nodiscard]] std::optional<v2> get_value_by_index(size_t index) const;
        [[nodiscard]] std::optional<f32> get_value_by_x(f32 x) const;

        [[nodiscard]] v2 graph_min() const { return m_graph_min; }
        [[nodiscard]] v2 graph_max() const { return m_graph_max; }
        [[nodiscard]] v2 graph_size() const { return glm::abs(m_graph_min) + glm::abs(m_graph_max); }

        void set_graph_min(v2 graph_min);
        void set_graph_max(v2 graph_max);

        void add_point(v2 point);
        void remove_point(size_t index);
        void edit_point(size_t& inout_index, v2 point);

    private:

        // Sort the input point from its index, adjusting the index to
        // reflect its new position.
        void sort(size_t& inout_index);

        void clamp_to_graph();
        void clamp_to_graph(size_t point_index);

    private:

        v2 m_graph_min{}, m_graph_max{};
        std::vector<v2> m_points{};

    };
}

namespace YAML
{
    inline Emitter& operator<<(Emitter& out, const h2o::Graph& graph)
    {
        out << YAML::BeginMap;

        out << YAML::Key << "points" << YAML::Value;
        out << YAML::BeginSeq;
        for (size_t i = 0; i < graph.num_points(); i++)
        {
            if (const auto point = graph.get_value_by_index(i))
                out << *point;
        }
        out << YAML::EndSeq;

        out << YAML::Key << "min" << YAML::Value << graph.graph_min();
        out << YAML::Key << "max" << YAML::Value << graph.graph_max();

        out << YAML::EndMap;

        return out;
    }

    template<>
    struct convert<h2o::Graph>
    {
        static bool decode(const Node& node, h2o::Graph& rhs)
        {
            rhs.set_graph_min(node["min"].as<v2>());
            rhs.set_graph_max(node["max"].as<v2>());

            const auto& points_yml = node["points"];
            for (const auto& point_yml : points_yml)
                rhs.add_point(point_yml.as<v2>());

            return true;
        }
    };
}