#pragma once

#include "core/handle_types.h"

#include <memory>
#include <string_view>

namespace h2o
{
    class Actor;
    class Component;

    struct ComponentInitializer
    {
        WeakHandle<Actor> owner;
    };

    class Component
    {
    public:

        explicit Component(const ComponentInitializer& component_initializer);
        Component(const Component&) = delete;
        Component(Component&&) = delete;
        virtual ~Component() = default;

        /**
         * Get this component's owner actor
         *
         * @tparam T The actor's type
         * @return A pointer to this component's owner actor
         */
        template<class T = Actor>
        WeakHandle<T> owner() const;

    private:

        WeakHandle<Actor> m_owner;

    };

    template<class T>
    WeakHandle<T> Component::owner() const
    {
        return oup::dynamic_pointer_cast<T>(m_owner);
    }
}