#include "core/engine.h"

#include "core/log.h"
#include "core/module.h"

#include <iostream>
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
        init_new_modules();

        while (!m_window_module || !m_window_module->should_close())
            update();
    }

    void Engine::register_tickable(Tickable& tickable, TickPhase phases)
    {
        auto phase = static_cast<TickPhase>(1);
        for (auto& tickables : m_tickables)
        {
            if (phase & phases)
                tickables.push_back(&tickable);

            phase = phase << 1;
        }
    }

    void Engine::unregister_tickable(Tickable& tickable, TickPhase phases)
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

        f64 delta_time = 0.0f;
        if (m_window_module)
        {
            m_window_module->poll_events();
            delta_time = m_window_module->delta_time();
        }

        auto run_tick = [&](
                const TickPhase tick_phase,
                void(Tickable::*tick_function)(f32))
            {
                auto phase_idx = magic_enum::enum_index(tick_phase);
                if (phase_idx.has_value())
                {
                    for (const auto& tickable: m_tickables[*phase_idx])
                        (tickable->*tick_function)(static_cast<f32>(delta_time));
                }
            };

        run_tick(FrameStart, &Tickable::frame_start);

        run_tick(Update,     &Tickable::update);
        run_tick(PreRender,  &Tickable::pre_render);
        run_tick(Render,     &Tickable::render);
        run_tick(PostRender, &Tickable::post_render);

        run_tick(FrameEnd,   &Tickable::frame_end);

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