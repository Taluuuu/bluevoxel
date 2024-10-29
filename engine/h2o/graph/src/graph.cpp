#include "graph/graph.h"

namespace h2o
{
    Graph::Graph(const v2 graph_min, const v2 graph_max)
        : m_graph_min(graph_min), m_graph_max(graph_max)
    {}

    std::optional<v2> Graph::get_value_by_index(const size_t index) const
    {
        if (index < m_points.size())
            return m_points[index];

        return std::nullopt;
    }

    std::optional<f32> Graph::get_value_by_x(const f32 x) const
    {
        if (m_points.empty())
            return std::nullopt;

        for (size_t i = 0; i < m_points.size() - 1; i++)
        {
            const v2 min = m_points[i];
            const v2 max = m_points[i + 1];

            if (min.x <= x && max.x >= x)
            {
                const f32 dist_x = max.x - min.x;
                if (dist_x < 0.000001f)
                    continue; // Prevent division by 0

                const f32 t = (x - min.x) / dist_x;
                return glm::mix(min.y, max.y, t);
            }
        }

        return std::nullopt;
    }

    void Graph::set_graph_min(const v2 graph_min)
    {
        m_graph_min = graph_min;
        clamp_to_graph();
    }

    void Graph::set_graph_max(const v2 graph_max)
    {
        m_graph_max = graph_max;
        clamp_to_graph();
    }

    void Graph::add_point(const v2 point)
    {
        size_t index = m_points.size();
        m_points.push_back(point);

        clamp_to_graph(index);
        sort(index);
    }

    void Graph::remove_point(const size_t index)
    {
        if (index < m_points.size())
            m_points.erase(m_points.begin() + static_cast<long>(index));
    }

    void Graph::edit_point(size_t& inout_index, const v2 point)
    {
        if (inout_index < m_points.size())
        {
            m_points[inout_index] = point;

            clamp_to_graph(inout_index);
            sort(inout_index);
        }
    }

    void Graph::sort(size_t& inout_index)
    {
        if (m_points.size() <= 1 || inout_index >= m_points.size())
            return;

        const v2 point = m_points[inout_index];
        remove_point(inout_index);

        for (size_t i = 0; i < m_points.size(); i++)
        {
            if (point.x < m_points[i].x)
            {
                m_points.insert(m_points.begin() + static_cast<long>(i), point);
                inout_index = i;
                return;
            }
        }

        inout_index = m_points.size();
        m_points.push_back(point);
    }

    void Graph::clamp_to_graph()
    {
        for (size_t i = 0; i < num_points(); i++)
            clamp_to_graph(i);
    }

    void Graph::clamp_to_graph(const size_t point_index)
    {
        if (point_index < m_points.size())
        {
            v2& point = m_points[point_index];
            point = glm::clamp(point, m_graph_min, m_graph_max);
        }
    }
}