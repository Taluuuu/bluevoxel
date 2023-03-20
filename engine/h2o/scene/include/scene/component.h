#pragma once

#include <memory>
#include <string_view>

namespace h2o
{
    class Actor;
    class Component;

    struct ComponentInitializer
    {
        const std::shared_ptr<Actor>& owner;
    };

    template<class T>
    class CachedComponent
    {
    public:

        CachedComponent(T& component);
        CachedComponent(const CachedComponent&) = delete;
        CachedComponent(CachedComponent&&) = delete;
        ~CachedComponent();

    private:

        T* const m_component = nullptr;

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

        std::vector<CachedComponent<Component>*> m_observers;

        Actor* m_owner;

    };

    template<class T>
    CachedComponent<T>::CachedComponent(T& component)
        : m_component(&component)
    {
        //component.
    }

    template<class T>
    CachedComponent<T>::~CachedComponent()
    {
        assert(m_component);

    }

    template<class T>
    T* Component::get_owner()
    {
        return dynamic_cast<T*>(m_owner);
    }
}