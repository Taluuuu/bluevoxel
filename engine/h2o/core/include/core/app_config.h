#pragma once

#include "core/log.h"
#include "core/types.h"

#include <cassert>
#include <string>
#include <unordered_map>
#include <variant>

namespace h2o
{
    class AppConfig
    {
    public:

        AppConfig() = default;
        ~AppConfig();

        void load_file();
        void save_file();

        template<typename T>
        [[nodiscard]] T get(const std::string& name) const;

        // This must be called for every expected entry in the config file
        // before loading the app config file.
        template<typename T>
        void register_entry(const std::string& name, const T& default_value);

        static constexpr std::string config_file_name = "app_config.yml";
        using EntryType = std::variant<bool, f32, i32, std::string>;

    private:

        bool m_can_save = true;
        std::unordered_map<std::string, EntryType> m_config_entries{};

    };

    template <typename T>
    T AppConfig::get(const std::string& name) const
    {
        const auto it = m_config_entries.find(name);
        assert(it != m_config_entries.end() && "Config entry not registered.");

        if (const T* val = std::get_if<T>(&it->second))
            return *val;

        assert(false && "Config entry type does not match expectation.");
        return T{};
    }

    template <typename T>
    void AppConfig::register_entry(const std::string& name, const T& default_value)
    {
        m_config_entries[name] = default_value;
    }
}
