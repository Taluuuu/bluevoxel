#pragma once

namespace h2o
{
    class Engine;

    class IUIRenderer
    {
    public:

        virtual ~IUIRenderer() = default;

        virtual bool init(Engine& engine) = 0;
        virtual void cleanup() = 0;

    };
}