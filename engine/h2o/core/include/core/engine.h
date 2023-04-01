#pragma once

#include "game_info.h"
#include "core/core_interfaces.h" // Could probably be removed

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <stack>

namespace h2o
{
    class IModule;

    class Engine
    {
    public:

        explicit Engine(const GameInfo& game_info);
        Engine(const Engine&) = delete;
        Engine(Engine&&) = delete;
        ~Engine();

        /**
         * @brief Add a module to the engine. Initialize it once all its dependencies
         *        are met.
         * 
         * @tparam T The module's type
         * @tparam Args The module's constructor argument types
         * @param args The module's constructor arguments
         * @return A reference to the engine used to chain add_module calls
         */
        template<typename T, typename... Args>
        Engine& add_module(Args... args);

        /**
         * @brief Get the module of type T if it was correctly initialized
         * 
         * @tparam T The module's type
         * @return A pointer to the module or nullptr if it was not found
         */
        template<typename T>
        T* get_module() const;

        [[nodiscard]] const GameInfo& game_info() const { return m_game_info; }

        void register_tickable(ITickable* tickable, TickPhase tick_phases);

        /**
         * @brief Run the engine. Contains the main loop.
         * 
         */
        void run();

    private:
    
        void update() const;

        void init_new_modules();

    private:

        GameInfo m_game_info;

        // Modules to initialize
        std::unordered_map<std::type_index, std::unique_ptr<IModule>> m_modules_to_init;

        // Initialized modules
        std::unordered_map<std::type_index, IModule*> m_initialized_modules;
        std::stack<std::unique_ptr<IModule>> m_module_stack;

        std::vector< std::vector<ITickable*> > m_tickables;

        // Queried interfaces
        IWindowModule* m_window_module = nullptr;
        IInputModule*  m_input_module  = nullptr;

    };

    template<typename T, typename... Args>
    Engine& Engine::add_module(Args... args)
    {
        static_assert(std::is_base_of_v<IModule, T>, "T must implement h2o::IModule.");

        std::type_index module_type(typeid(T));

        // Make sure the module hadn't already been added
        assert( !m_initialized_modules.contains(module_type) &&
            !m_modules_to_init.contains(module_type) );

        m_modules_to_init[module_type] = std::make_unique<T>(args...);

        return *this;
    }

    template<typename T>
    T* Engine::get_module() const
    {
        auto it = m_initialized_modules.find(typeid(T));
        if (it == m_initialized_modules.end())
            return nullptr;

        return dynamic_cast<T*>(it->second);
    }
}

/**
 * Global engine instance
 */
inline h2o::Engine* g_engine = nullptr;