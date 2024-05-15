#pragma once

#include "app_config.h"
#include "game_info.h"
#include "resources.h"
#include "thread_pool.h"
#include "tickable.h"

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <stack>

namespace h2o
{
    class IModule;

    class Engine : public Tickable
    {
    public:

        explicit Engine(const GameInfo& game_info);
        Engine(const Engine&) = delete;
        Engine(Engine&&) = delete;
        ~Engine() override;

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
        [[nodiscard]] T* get_module() const;

        /**
         * @brief Get the module of type T if it was correctly initialized.
         *        Asserts that the module is valid.
         *
         * @tparam T The module's type
         * @return A reference to the module
         */
        template<typename T>
        [[nodiscard]] T& get_module_checked() const;

        [[nodiscard]] const GameInfo& game_info() const { return m_game_info; }
        [[nodiscard]] const AppConfig& app_config() const { return m_app_config; }
        [[nodiscard]] ResourceManager& resource_mgr() { return m_resource_mgr; }
        [[nodiscard]] ThreadPool& thread_pool() { return m_thread_pool; }

        /**
         * @brief Run the engine. Contains the main loop.
         *
         */
        void run();

    public:

        bool should_close = false;

    private:

        void update();

        bool init_modules();

    private:

        GameInfo m_game_info{};
        AppConfig m_app_config{};

        // Modules to initialize
        std::unordered_map<std::type_index, std::unique_ptr<IModule>> m_modules_to_init;

        // Initialized modules
        std::unordered_map<std::type_index, IModule*> m_initialized_modules;
        std::stack<std::unique_ptr<IModule>> m_module_stack;

        // Queried interfaces
        class IWindowModule* m_window_module = nullptr;
        class IInputModule*  m_input_module  = nullptr;

        f32 m_time_since_network_update = 0.0f;
        f32 m_time_between_network_updates = 1.0f / 20.0f;

        ResourceManager m_resource_mgr{};

        ThreadPool m_thread_pool{};

    };

    template<typename T, typename... Args>
    Engine& Engine::add_module(Args... args)
    {
        static_assert(std::is_base_of_v<IModule, T>, "T must implement h2o::IModule.");

        std::type_index module_type(typeid(T));

        // Make sure the module hadn't already been added
        assert(!m_initialized_modules.contains(module_type));
        assert(!m_modules_to_init.contains(module_type));

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

    template<typename T>
    T& Engine::get_module_checked() const
    {
        T* module = get_module<T>();
        assert(module != nullptr);

        return *module;
    }
}

/**
 * Global engine instance
 */
inline h2o::Engine* g_engine = nullptr;