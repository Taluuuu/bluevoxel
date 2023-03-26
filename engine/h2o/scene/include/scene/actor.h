#pragma once

#include "core/log.h"
#include "core/types.h"
#include "core/handle_types.h"
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
        WeakHandle<T> get_component();

        /**
         * Add a component of type T to this actor
         *
         * @tparam T The component's type, deriving from h2o::Component
         * @tparam Args The component's constructor argument types
         * @param args The component's constructor arguments
         * @return The created component or nullptr on failure
         */
        template<class T, typename... Args>
        WeakHandle<T> add_component(Args... args);

        // Temporary, will be replaced by a better ticking system
        virtual void tick(f32 delta_time) {}

    private:

        std::unordered_map< std::type_index, OwningHandle<Component> > m_components;

        std::string_view m_name;

        std::shared_ptr<Scene> m_scene;

    };

    template<class T>
    WeakHandle<T> Actor::get_component()
    {
        static_assert(
            std::is_base_of_v<Component, T>,
            "T must derive from h2o::Component.");

        auto comp_it = m_components.find(typeid(T));
        return (comp_it == m_components.end()) ?
            nullptr :
            oup::dynamic_pointer_cast<T>(WeakHandle<Component>(comp_it->second));
    }

    template<class T, typename... Args>
    WeakHandle<T> Actor::add_component(Args... args)
    {
        static_assert(
            std::is_base_of_v<Component, T> && !std::is_same_v<Component, T>,
            "T must derive from h2o::Component.");

        if (auto comp = get_component<T>())
        {
            log::warn("Only one instance of a component can be added to an actor ({}).", m_name);
            return comp;
        }

        ComponentInitializer component_initializer
        {
            .owner = this
        };

        OwningHandle<T> new_comp(new T(component_initializer, args...));
        WeakHandle<T> weak_comp_handle = new_comp;

        m_components.insert({
            typeid(T),
            std::move(new_comp) });

        return weak_comp_handle;
    }
}