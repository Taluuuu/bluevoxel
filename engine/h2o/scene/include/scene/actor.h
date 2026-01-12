#pragma once

#include "actor_initializer.h"
#include "core/log.h"
#include "core/types.h"
#include "core/handle_types.h"
#include "core/tickable.h"
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

    class Actor
        : public Tickable
        , public oup::enable_observer_from_this_unique<Actor>
    {
    public:

        explicit Actor(const ActorInitializer& actor_initializer);
        Actor(const Actor&) = delete;
        Actor(Actor&&) = delete;
        ~Actor() override = default;

        // Ran before the first update this actor is alive
        virtual void start();

        /**
         * Get the component of type T on this actor
         *
         * @tparam T The type of component, must derive from h2o::Component
         * @return A pointer to the component instance or nullptr if none was found
         */
        template<class T>
        requires (std::derived_from<T, Component> && !std::same_as<Component, T>)
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
        requires (std::derived_from<T, Component> && !std::same_as<Component, T>)
        WeakHandle<T> add_component(Args... args);

        [[nodiscard]] ActorTag get_tag() const { return m_actor_tag; }
        void tag_actor(ActorTag tag);

        void set_replicate_transform(bool replicate);

    public:

        Transform transform;

        friend class Scene;
        Scene& scene;

    protected:

        ActorTag m_actor_tag = ActorTag::None;

        const ActorID m_actor_id = 0;


    private:

        std::unordered_map< std::type_index, OwningHandle<Component> > m_components;

    };

    template<class T>
    requires (std::derived_from<T, Component> && !std::same_as<Component, T>)
    WeakHandle<T> Actor::get_component()
    {
        // No clue how slow this is but it is the only way I could find at 2:12 AM to allow getting
        // a child component and not a component of just type T. I left the old implementation under
        // for future reference.
        for (const auto& [_, component] : m_components)
        {
            if (auto comp = oup::dynamic_pointer_cast<T>(WeakHandle<Component>(component)))
                return comp;
        }

        return nullptr;

//        auto comp_it = m_components.find(typeid(T));
//        return (comp_it == m_components.end()) ?
//            nullptr :
//            oup::dynamic_pointer_cast<T>(WeakHandle<Component>(comp_it->second));
    }

    template<class T, typename... Args>
    requires (std::derived_from<T, Component> && !std::same_as<Component, T>)
    WeakHandle<T> Actor::add_component(Args... args)
    {
        if (auto comp = get_component<T>())
        {
            log::warn("Only one instance of a component can be added to an actor.");
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