#pragma once

#include "core/log.h"

#include <optional>
#include <yaml-cpp/node/node.h>

namespace h2o
{
    class AppConfig
    {
    public:

        static std::optional<AppConfig> load();

        template<typename T>
        std::optional<T> get(const std::string& name) const;

        static constexpr std::string config_file_name = "app_config.yml";

    protected:

        explicit AppConfig(const YAML::Node& config_root);

    private:

        YAML::Node m_config_root{};

    };

    template <typename T>
    std::optional<T> AppConfig::get(const std::string& name) const
    {
        try
        {
            return m_config_root[name].as<T>();
        }
        catch(const std::exception& e)
        {
            log::warn("Failed to read {} in config file: {}", name, e.what());
            return std::nullopt;
        }
    }
}
