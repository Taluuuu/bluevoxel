#include "core/engine.h"

namespace engine
{
    void Engine::cleanup()
    {
        // TODO: This probably needs to be ordered
        for (auto& module : m_modules)
        {
            assert(module.second != nullptr);
            module.second->cleanup();
        }
    }

    bool Engine::loaded_all_modules() const
    {
        return m_uninitialized_modules.empty();
    }

    bool Engine::is_module_added(const std::type_index& module_type) const
    {
        return  m_modules.contains(module_type) ||
                m_uninitialized_modules.contains(module_type);
    }

    bool Engine::module_can_be_loaded(const std::shared_ptr<Module>& module) const
    {
        for (const auto& dep : module->get_dependencies())
        {
            if (!m_modules.contains(std::type_index(dep)))
                return false;
        }

        return true;
    }
    
    void Engine::init_new_modules()
    {
        while (true)
        {
            const auto& init_modules = m_modules;
            auto it = std::find_if(m_uninitialized_modules.begin(), m_uninitialized_modules.end(), 
                [&init_modules](const auto& module)
                {
                    for (const auto& module_dep : module.second->get_dependencies())
                    {
                        if (!init_modules.contains(module_dep))
                            return false;
                    }
                    
                    return true;
                }
            );

            if (it == m_uninitialized_modules.end())
                return;

            if (it->second->init())
            {
                // Add module to initialized list
                m_modules[it->first] = it->second;
                m_uninitialized_modules.erase(it->first);
            }
            else
            {
                std::cout 
                    << "[Error] Failed to initialize module: '" 
                    << it->second->get_module_name() << "'\n";

                // Don't try loading the module in subsequent runs
                m_uninitialized_modules.erase(it->first);
            }
        }
    }
}