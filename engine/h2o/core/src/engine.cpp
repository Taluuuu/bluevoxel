#include "core/engine.h"

#include "core/log.h"
#include "core/module.h"

#include <iostream>
#include <magic_enum.hpp>
#include <ranges>

namespace h2o
{
    Engine::Engine(const GameInfo& game_info)
        : m_game_info(game_info)
    {
        m_tickables.resize(magic_enum::enum_count<TickPhase>(), {});
    }

    Engine::~Engine()
    {
        while (!m_module_stack.empty())
        {
            auto& module = m_module_stack.top();
            module->cleanup();
            m_module_stack.pop();
        }
    }

    void Engine::register_tickable(ITickable* tickable, TickPhase tick_phases)
    {
        for (u32 i = 0; i < magic_enum::enum_count<TickPhase>(); i++)
        {
            auto tick_phase = static_cast<TickPhase>(1 << i);
            auto& tickables = m_tickables[i];

            std::erase(tickables, tickable);

            if (tick_phase & tick_phases)
                tickables.push_back(tickable);
        }
    }

    void Engine::run()
    {
        init_new_modules();

        while (!m_window_module || !m_window_module->should_close())
            update();
    }

    void Engine::update() const
    {
        if (m_input_module)
        {
            m_input_module->prepare();
        }

        f64 delta_time = 0.0f;
        if (m_window_module)
        {
            m_window_module->poll_events();
            delta_time = m_window_module->delta_time();
        }

        TickPhase tick_phase = static_cast<TickPhase>(1);
        for (const auto& modules_to_tick : m_tickables)
        {
            for (const auto& module_to_tick : modules_to_tick)
                module_to_tick->tick(tick_phase, delta_time);

            tick_phase = static_cast<TickPhase>(tick_phase << 1);
        }

        if (m_window_module)
        {
            m_window_module->swap_buffers(144.0);
        }
    }

    void Engine::init_new_modules()
    {
        while (true)
        {
            // Find next module to init
            auto it = std::find_if(m_modules_to_init.begin(), m_modules_to_init.end(),
                [this](const auto& module)
                {
                    for (const auto& module_dep : module.second->dependencies())
                    {
                        if (!m_initialized_modules.contains(module_dep))
                            return false;
                    }
                    
                    return true;
                });

            if (it == m_modules_to_init.end())
            {
                // No more modules can be initialized
                return;
            }

            if (it->second->init(*this))
            {
                // Release the unique_ptr
                auto module = it->second.release();

                // Take ownership of the module ptr
                m_module_stack.push(std::unique_ptr<IModule>(module));
                m_initialized_modules[it->first] = module;

                // Remove from modules to init
                m_modules_to_init.erase(it);

                // Query interfaces...
                if (auto window_module = dynamic_cast<IWindowModule*>(module))
                {
                    assert(!m_window_module);
                    m_window_module = window_module;
                }
                if (auto input_module = dynamic_cast<IInputModule*>(module))
                {
                    assert(!m_input_module);
                    m_input_module = input_module;
                }
            }
            else
            {
                log::error("Failed to initialize module '{}'", it->second->module_name());

                // Don't try loading the module in subsequent runs
                m_modules_to_init.erase(it->first);
            }
        }
    }
}

h2o::Engine* g_engine = nullptr;