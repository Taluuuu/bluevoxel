#include "core/debug_infos.h"

namespace h2o
{
    void DebugInfos::update_debug_statistic(
        const std::string& category,
        const std::string& name,
        const DebugStatType& value)
    {
        auto it = m_debug_stats_by_category.find(category);
        if (it == m_debug_stats_by_category.end())
        {
            const auto[inserted_it, success] =
                m_debug_stats_by_category.emplace(category, std::vector<DebugStat>{});

            if (!success)
                return;

            it = inserted_it;
        }

        auto& stats = it->second;
        for (auto& stat : stats)
        {
            if (stat.name == name)
            {
                stat.value = value;
                return;
            }
        }

        // Stat does not yet exist
        stats.emplace_back(name, value);
    }
}
