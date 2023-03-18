#pragma once

#include "core/log.h"

#include <memory>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace h2o
{
    class Actor;
    class Component;
    class Scene;

    struct ActorInitializer
    {
        std::string_view actor_name;
        const std::shared_ptr<Scene> scene;
    };

    struct ComponentInitializer
    {
        const std::shared_ptr<Actor> owner;
    };

    class Actor : public std::enable_shared_from_this<Actor>
    {
    public:

        explicit Actor(const ActorInitializer& actor_initializer);
        Actor(const Actor&) = delete;
        Actor(Actor&&) = delete;

        template<typename T>
        T* get_component()
        {
            auto comp_it = m_components.find(typeid(T));
            return (comp_it == m_components.end()) ? nullptr : comp_it->second.get();
        }

        template<typename T, typename... Args>
        void add_component(Args... args)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from h2o::Component.");

            if (get_component<T>() != nullptr)
            {
                log::warn("Only one instance of a component can be added to an actor ({}).", m_name);
                return;
            }

            ComponentInitializer component_initializer
            {
                .owner = shared_from_this()
            };

            m_components.insert({ typeid(T), std::make_unique<T>(component_initializer, args...) });
        }

    private:

        std::unordered_map< std::type_index, std::unique_ptr<Component> > m_components;

        std::string_view m_name;

        std::shared_ptr<Scene> m_scene;

    };
}