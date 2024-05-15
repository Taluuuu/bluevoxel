#include "core/app_config.h"

namespace h2o
{
    AppConfig::AppConfig()
    {
        try
        {
            m_config_root = YAML::LoadFile(config_file_name);
        }
        catch (const std::exception& e)
        {
            log::warn("Did not find app config file: {}", e.what());
        }
    }
}
