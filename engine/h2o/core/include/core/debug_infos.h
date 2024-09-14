#pragma once

#include "core/types.h"

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace h2o
{
    using DebugStatType = std::variant<i32, f32>;

    class DebugInfos
    {
    public:

        void update_debug_statistic(
            const std::string& category,
            const std::string& name,
            const DebugStatType& value);

        struct DebugStat
        {
            std::string name{};
            DebugStatType value{};
        };

        using CategoryStatsMap = std::map<std::string, std::vector<DebugStat>>;

        [[nodiscard]] const CategoryStatsMap& debug_stats_by_category() const
        { return m_debug_stats_by_category; }

    private:

        CategoryStatsMap m_debug_stats_by_category{};

    };
}
