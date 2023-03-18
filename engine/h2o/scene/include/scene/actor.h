#pragma once

#include "core/log.h"
#include "scene/component.h"

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
        const std::shared_ptr<Scene>& scene;
    };

    class Actor : public std::enable_shared_from_this<Actor>
    {
    public:

        explicit Actor(const ActorInitializer& actor_initializer);
        Actor(const Actor&) = delete;
        Actor(Actor&&) = delete;
        virtual ~Actor() = default;

        /**
         * Get the component of type T on this actor
         *
         * @tparam T The type of component, must derive from h2o::Component
         * @return A pointer to the component instance or nullptr if none was found
         */
        template<class T>
        T* get_component();

        /**
         * Add a component of type T to this actor
         *
         * @tparam T
         * @tparam Args
         * @param args
         * @return
         */
        template<class T, typename... Args>
        T* add_component(Args... args);

    private:

        std::unordered_map< std::type_index, std::unique_ptr<Component> > m_components;

        std::string_view m_name;

        std::shared_ptr<Scene> m_scene;

    };

    template<class T>
    T* Actor::get_component()
    {
        auto comp_it = m_components.find(typeid(T));
        return (comp_it == m_components.end()) ?
            nullptr :
            dynamic_cast<T*>(comp_it->second.get());
    }

    template<class T, typename... Args>
    T* Actor::add_component(Args... args)
    {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from h2o::Component.");

        if (get_component<T>() != nullptr)
        {
            log::warn("Only one instance of a component can be added to an actor ({}).", m_name);
            return nullptr;
        }

        ComponentInitializer component_initializer
        {
            .owner = shared_from_this()
        };

        auto new_comp = new T(component_initializer, args...);

        m_components.insert({
            typeid(T),
            std::unique_ptr<T>(new_comp) });

        return new_comp;
    }
}