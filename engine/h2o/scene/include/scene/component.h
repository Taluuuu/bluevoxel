#pragma once

#include <memory>
#include <string_view>

namespace h2o
{
    class Actor;

    struct ComponentInitializer
    {
        const std::shared_ptr<Actor>& owner;
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
        T* get_owner();

    private:

        Actor* m_owner;

    };

    template<class T>
    T* Component::get_owner()
    {
        return dynamic_cast<T*>(m_owner);
    }
}