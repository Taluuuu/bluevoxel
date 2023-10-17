#include "core/engine.h"

#include "core/log.h"
#include "core/module.h"
#include "core/core_interfaces.h"

#include <algorithm>
#include <magic_enum.hpp>

namespace h2o
{
    Engine::Engine(const GameInfo& game_info)
        : m_game_info(game_info)
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
        setvbuf(stdout, NULL, _IONBF, 0);

        m_thread_pool.start();

        init_new_modules();

        while (!should_close)
            update();

        m_thread_pool.stop();
    }

    void Engine::register_tickable(Tickable& tickable, TickPhase::Type phases)
    {
        auto phase = static_cast<TickPhase::Type>(1);
        for (auto& tickables : m_tickables)
        {
            if (phase & phases)
                tickables.push_back(&tickable);

            phase = phase << 1;
        }
    }

    void Engine::unregister_tickable(Tickable& tickable, TickPhase::Type phases)
    {
        u32 index = 1;
        for (auto& tickables : m_tickables)
        {
            if (index & phases)
                std::erase(tickables, &tickable);

            index = index << 1;
        }
    }

    void Engine::update() const
    {
        if (m_input_module)
            m_input_module->prepare();

        f32 delta_time = 0.0f;
        if (m_window_module)
        {
            m_window_module->poll_events();
            delta_time = static_cast<f32>(m_window_module->delta_time());
        }

        auto run_tick = [&](
                const TickPhase::Type tick_phase,
                void(Tickable::*tick_function)(f32),
                f32 delta_time)
            {
                if (auto phase_idx = magic_enum::enum_index(tick_phase); phase_idx.has_value())
                {
                    const auto& tickables = m_tickables[*phase_idx];
                    for (size_t i = 0; i < tickables.size(); i++)
                        (tickables[i]->*tick_function)(static_cast<f32>(delta_time));
                }
            };

        run_tick(TickPhase::FrameStart, &Tickable::frame_start, delta_time);

        if (true)
        {
            run_tick(TickPhase::NetworkUpdate, &Tickable::network_update, delta_time);
        }

        run_tick(TickPhase::Update,     &Tickable::update, delta_time);
        run_tick(TickPhase::PreRender,  &Tickable::pre_render, delta_time);
        run_tick(TickPhase::Render,     &Tickable::render, delta_time);
        run_tick(TickPhase::PostRender, &Tickable::post_render, delta_time);

        run_tick(TickPhase::FrameEnd,   &Tickable::frame_end, delta_time);

        if (m_window_module)
            m_window_module->swap_buffers(144.0);
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
                break;
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

        for (const auto& module : m_modules_to_init)
        {
            log::warn("Could not initialize all dependencies for module: '{}'", module.second->module_name());
        }
    }
}