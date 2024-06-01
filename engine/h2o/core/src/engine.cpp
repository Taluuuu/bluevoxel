#include "core/engine.h"

#include "core/log.h"
#include "core/module.h"
#include "core/core_interfaces.h"

#include <algorithm>
#include <magic_enum.hpp>

namespace h2o
{
    Engine::Engine(const GameInfo& game_info)
        : Tickable(nullptr)
        , m_game_info(game_info)
    {
        assert(!g_engine);
        g_engine = this;
    }

    Engine::~Engine()
    {
        // Stop holding resources
        m_resource_mgr.unload_all();

        m_modules_to_init.clear();

        while (!m_module_stack.empty())
        {
            auto& module = m_module_stack.top();
            module->cleanup();
            m_module_stack.pop();
        }

        assert(g_engine);
        g_engine = nullptr;
    }

    void Engine::run()
    {
        // Fixes output in debug in CLion
        setvbuf(stdout, nullptr, _IONBF, 0);

        if (!init_modules())
            return;

        m_thread_pool.start();

        m_app_config.load_file();

        while (!should_close)
            update();

        m_thread_pool.stop();
    }

    void Engine::update()
    {
        if (m_input_module)
            m_input_module->prepare();

        f32 delta_time = 0.0f;
        if (m_window_module)
        {
            m_window_module->poll_events();
            delta_time = static_cast<f32>(m_window_module->delta_time());
        }

        run_frame_start(delta_time);

        m_time_since_network_update += delta_time;
        if (m_time_since_network_update > m_time_between_network_updates)
        {
            run_network_update(delta_time);
            m_time_since_network_update = 0.0f;
        }

        run_update(delta_time);
        run_post_update(delta_time);

        run_pre_render();
        run_render();
        run_post_render();

        run_frame_end(delta_time);

        if (m_window_module)
            m_window_module->swap_buffers(144.0);
    }

    bool Engine::init_modules()
    {
        const auto init_module = [&](const std::type_index& module_type, IModule& module) -> bool
        {
            if (module.init(*this))
            {
                // Take ownership of the module ptr
                m_module_stack.emplace(&module);
                m_initialized_modules[module_type] = &module;

                // Remove from modules to init
                m_modules_to_init.erase(module_type);

                return true;
            }

            log::error("Failed to initialize module '{}'", module.module_name());
            return false;
        };

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

                    for (const auto& module_dep : module.second->optional_dependencies())
                    {
                        if (!m_initialized_modules.contains(module_dep))
                            return false;
                    }
                    
                    return true;
                }
            );

            if (it == m_modules_to_init.end())
            {
                // No more modules can be initialized
                break;
            }

            if (!init_module(it->first, *it->second.release()))
                return false;
        }

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
                }
            );

            if (it == m_modules_to_init.end())
            {
                // No more modules can be initialized
                break;
            }

            if (!init_module(it->first, *it->second.release()))
                return false;
        }

        for (const auto& module : m_modules_to_init)
        {
            log::warn("Could not initialize all dependencies for module: '{}'", module.second->module_name());
            return false;
        }

        for (const auto& [_, module] : m_initialized_modules)
        {
            // Query interfaces...
            // TODO: These should go, modules should do stuff by themselves.
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

        return true;
    }
}