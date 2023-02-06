#include "core/engine.h"

#include "core/module.h"

#include <iostream>

namespace engine
{
    Engine* Engine::s_instance = nullptr;

    Engine::Engine(const GameInfo& game_info)
        :m_game_info(game_info)
    {
        assert(s_instance == nullptr);
        s_instance = this;
    }

    Engine::~Engine()
    {
        s_instance = nullptr;
    }

    void Engine::run() const
    {
        while (!m_core_window || !m_core_window->should_close())
            update();

        cleanup();
    }

    void Engine::update() const
    {
        f64 delta_time = m_core_window ? 
            m_core_window->delta_time() : 0.0;

        if (m_core_window != nullptr)
            m_core_window->poll_events();

        for (const auto& tickable : m_tickables)
            tickable->tick(delta_time);

        if (m_core_window != nullptr)
            m_core_window->swap_buffers();
    }

    void Engine::cleanup() const
    {
        // Cleanup in reverse order from initialization
        for (auto i = m_ordered_modules.rbegin(); i != m_ordered_modules.rend(); i++)
        {
            assert(*i != nullptr);
            (*i)->cleanup();
        }
    }

    void Engine::init_new_modules()
    {
        while (true)
        {
            const auto& init_modules = m_initialized_modules;
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

            if (it->second->init(m_game_info))
            {
                auto module = it->second;

                // Add module to initialized list
                m_initialized_modules[it->first] = module;
                m_ordered_modules.push_back(module);
                m_uninitialized_modules.erase(it->first);

                // Query interfaces...
                if (auto dtp = std::dynamic_pointer_cast<IWindowModule>(module))
                    m_core_window = dtp;

                if (auto tickable = std::dynamic_pointer_cast<ITickable>(module))
                    m_tickables.push_back(tickable);
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