#include "core/app_config.h"

#include <yaml-cpp/node/parse.h>

namespace h2o
{
    std::optional<AppConfig> AppConfig::load()
    {
        try
        {
            const auto config_root = YAML::LoadFile(config_file_name);
            return AppConfig(config_root);
        }
        catch (const std::exception& e)
        {
            log::error("Failed to load app config file: {}", e.what());
            return std::nullopt;
        }
    }

    AppConfig::AppConfig(const YAML::Node& config_root)
        : m_config_root(config_root)
    {}
}
