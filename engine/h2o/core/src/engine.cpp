#include "core/engine.h"

#include "core/core_interfaces.h"
#include "core/log.h"
#include "core/module.h"
#include "core/time_provider_chrono.h"

#include <algorithm>
#include <magic_enum/magic_enum.hpp>

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

        for (auto& module : m_module_stack)
            module->on_engine_starts_closing();

        while (!m_module_stack.empty())
        {
            auto& module = m_module_stack.back();
            module->cleanup();
            m_module_stack.pop_back();
        }

        assert(g_engine);
        g_engine = nullptr;
    }

    const ITimeProvider& Engine::time_provider() const
    {
        assert(m_time_provider != nullptr);
        return *m_time_provider;
    }

    void Engine::run()
    {
        // Fixes output in debug in CLion
        setvbuf(stdout, nullptr, _IONBF, 0);

        if (!init_modules())
            return;

        if (!m_time_provider)
            m_time_provider = std::make_shared<TimeProvider_Chrono>();

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

        run_frame_start();

        const f32 delta_time = m_time_provider->delta_time();

        m_time_since_network_update += delta_time;
        if (m_time_since_network_update > m_time_between_network_updates)
        {
            run_network_update(delta_time);
            m_time_since_network_update = 0.0f;
            // log::info("Network update");
        }

        run_update(delta_time);
        run_physics_update(delta_time);
        run_post_update(delta_time);

        // General engine stats
        m_debug_infos.update_debug_statistic("engine", "queued job count",
            static_cast<i32>(m_thread_pool.job_count()));
        m_debug_infos.update_debug_statistic("engine", "thread count",
            static_cast<i32>(m_thread_pool.thread_count()));

        run_pre_render();
        run_render();
        run_post_render();

        run_frame_end(delta_time);
    }

    bool Engine::init_modules()
    {
        const auto init_module = [&](const std::type_index& module_type, IModule& module) -> bool
        {
            if (module.init(*this))
            {
                // Take ownership of the module ptr
                m_module_stack.emplace_back(&module);
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
            if (auto input_module = dynamic_cast<IInputModule*>(module))
            {
                assert(!m_input_module);
                m_input_module = input_module;
            }
        }

        return true;
    }
}