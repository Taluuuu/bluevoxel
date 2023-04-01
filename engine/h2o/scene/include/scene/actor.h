#pragma once

#include "core/log.h"
#include "core/types.h"
#include "core/handle_types.h"
#include "scene/component.h"
#include "transform.h"

#include <typeindex>
#include <unordered_map>
#include <vector>

namespace h2o
{
    class Scene;
    class Actor;
    class Component;

    struct ActorInitializer
    {
        std::string_view actor_name;
        Scene* const scene;
    };

    class Actor : public oup::enable_observer_from_this_unique<Actor>
    {
    public:

        explicit Actor(const ActorInitializer& actor_initializer);
        Actor(const Actor&) = delete;
        Actor(Actor&&) = delete;
        ~Actor() override = default;

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

        Scene* scene() const { return m_scene; }

    public:

        Transform transform;

    protected:

        const std::string_view m_name;

        Scene* const m_scene;

    private:

        std::unordered_map< std::type_index, OwningHandle<Component> > m_components;

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
            .owner = *this
        };

        OwningHandle<T> new_comp = oup::make_observable_unique<T>(component_initializer, args...);
        WeakHandle<T> weak_comp_handle = new_comp;

        m_components.insert({ typeid(T), std::move(new_comp) });

        return weak_comp_handle;
    }
}