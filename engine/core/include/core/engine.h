#pragma once

#include <cassert>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <unordered_map>

namespace engine
{
    class Module;

    class Engine
    {
    public:

        template<typename T, typename... Args>
        Engine& add_module(Args... args)
        {
            static_assert(std::is_base_of_v<Module, T>, "T must implement engine::IModule.");

            std::type_index module_type(typeid(T));
            assert(!is_module_added(module_type));

            auto module = std::make_shared<T>(args...);
            m_uninitialized_modules[module_type] = module;

            init_new_modules();

            return *this;
        }

        template<typename T>
        T* get_module() const
        {
            auto it = m_modules.find(typeid(T));
            if (it == m_modules.end())
                return nullptr;

            return dynamic_cast<T*>(it->second.get());
        }

        void cleanup();

        bool loaded_all_modules() const;

    private:

        bool is_module_added(const std::type_index& module_type) const;

        bool module_can_be_loaded(const std::shared_ptr<Module>& module) const;

        void init_new_modules();

    private:

        // Modules that are waiting to be initialized
        std::unordered_map<std::type_index, std::shared_ptr<Module>> m_uninitialized_modules;

        // Initialized modules
        std::unordered_map<std::type_index, std::shared_ptr<Module>> m_modules;

    };
}