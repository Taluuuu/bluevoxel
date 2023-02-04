#pragma once

#include "game_info.h"
#include "core/core_interfaces.h"

#include <cassert>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace engine
{
    class Module;

    class Engine
    {
    public:

        Engine(const GameInfo& game_info);
        Engine(const Engine&) = delete;
        Engine(Engine&&) = delete;
        ~Engine();

        static Engine* instance() { return s_instance; }

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
        Engine& add_module(Args... args)
        {
            static_assert(std::is_base_of_v<Module, T>, "T must implement engine::IModule.");

            std::type_index module_type(typeid(T));
            
            // Make sure the module hadn't already been added
            assert( !m_initialized_modules.contains(module_type) &&
                    !m_uninitialized_modules.contains(module_type) );

            auto module = std::make_shared<T>(*this, args...);
            m_uninitialized_modules[module_type] = module;

            init_new_modules();
            return *this;
        }

        /**
         * @brief Get the module of type T if it was correctly initialized
         * 
         * @tparam T The module's typ
         * @return A pointer to the module or nullptr if it was not found
         */
        template<typename T>
        T* get_module() const
        {
            auto it = m_initialized_modules.find(typeid(T));
            if (it == m_initialized_modules.end())
                return nullptr;

            return dynamic_cast<T*>(it->second.get());
        }

        /**
         * @brief Run the engine. Contains the main loop.
         * 
         */
        void run() const;

    private:
    
        void update() const;

        void cleanup() const;

        void init_new_modules();

    private:

        static Engine* s_instance /* = nullptr */;

        GameInfo m_game_info;

        // Modules that are waiting to be initialized
        std::unordered_map<std::type_index, std::shared_ptr<Module>> m_uninitialized_modules;

        // Initialized modules
        std::unordered_map<std::type_index, std::shared_ptr<Module>> m_initialized_modules;

        // Modules in order of initialization
        std::vector<std::shared_ptr<Module>> m_ordered_modules;

        // Interfaces
        std::shared_ptr<ICoreWindow> m_core_window = nullptr;
        std::vector<std::shared_ptr<ITickable>> m_tickables;

    };
}