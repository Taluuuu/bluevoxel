#pragma once

#include "core/handle_types.h"
#include "core/tickable.h"

#include <memory>
#include <string_view>

namespace h2o
{
    class Actor;
    class Component;
    class Scene;

    struct ComponentInitializer
    {
        Actor& owner;
    };

    class Component : public Tickable
    {
    public:

        explicit Component(const ComponentInitializer& component_initializer);
        Component(const Component&) = delete;
        Component(Component&&) = delete;
        ~Component() override = default;

        virtual void start() {}

    public:

        Actor& owner;
        Scene& scene;

    };
}