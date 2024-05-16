#include "core/app_config.h"

#include <fstream>
#include <optional>
#include <yaml-cpp/yaml.h>

namespace YAML
{
    using EntryType = h2o::AppConfig::EntryType;

    template<>
    struct convert<EntryType>
    {
        static Node encode(const EntryType& rhs);
        static std::optional<EntryType> decode(const Node& node, const EntryType& default_entry);
    };

    Node convert<EntryType>::encode(const EntryType& rhs)
    {
        if (const i32* val = std::get_if<i32>(&rhs))
            return Node{ *val };

        if (const f32* val = std::get_if<f32>(&rhs))
            return Node{ *val };

        if (const bool* val = std::get_if<bool>(&rhs))
            return Node{ *val };

        if (const std::string* val = std::get_if<std::string>(&rhs))
            return Node{ *val };

        return Node{};
    }

    std::optional<EntryType> convert<EntryType>::decode(const Node& node, const EntryType& default_entry)
    {
        if (std::holds_alternative<i32>(default_entry))
        {
            if (i32 val; convert<i32>::decode(node, val))
                return val;
        }

        if (std::holds_alternative<f32>(default_entry))
        {
            if (f32 val; convert<f32>::decode(node, val))
                return val;
        }

        if (std::holds_alternative<bool>(default_entry))
        {
            if (bool val; convert<bool>::decode(node, val))
                return val;
        }

        if (std::holds_alternative<std::string>(default_entry))
        {
            if (std::string val; convert<std::string>::decode(node, val))
                return val;
        }

        return std::nullopt;
    }
}

namespace h2o
{
    AppConfig::~AppConfig()
    {
        // TODO: Investigate this later, probably bad to do this in the destructor.
        save_file();
    }

    void AppConfig::load_file()
    {
        try
        {
            const auto root_node = YAML::LoadFile(config_file_name);
            for (auto& [key, entry_value] : m_config_entries)
            {
                if (!root_node[key])
                    continue;

                const auto loaded_entry = YAML::convert<EntryType>::decode(root_node[key], entry_value);
                if (!loaded_entry)
                {
                    log::warn("Mismatch between expected and loaded type for config entry '{}'. Using default value.", key);
                    continue;
                }

                entry_value = *loaded_entry;
            }
        }
        catch (const YAML::BadFile&)
        {
            log::info("No config file could be found. Creating one.");
            save_file();
        }
        catch (const std::exception& e)
        {
            log::warn("Failed to load config file: {}. Using default values.", e.what());
            m_can_save = false;
        }
    }

    void AppConfig::save_file()
    {
        if (!m_can_save)
            return;

        try
        {
            YAML::Emitter emitter{};
            emitter << YAML::BeginMap;

            for (const auto& [key, value] : m_config_entries)
                emitter << YAML::Key << key << YAML::Value << YAML::convert<EntryType>::encode(value);

            emitter << YAML::EndMap;

            std::ofstream fout(config_file_name);
            fout << emitter.c_str();
        }
        catch (const std::exception& e)
        {
            log::warn("Failed to save config file: {}.", e.what());
        }
    }
}
