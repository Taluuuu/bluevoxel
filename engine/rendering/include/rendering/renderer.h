#pragma once

#include <memory>

namespace engine
{
    class IPipeline;
    class IWindow;

    class IRenderer
    {
    public:

        virtual ~IRenderer() {}

        // TODO: Make this accessible only by the rendering module
        virtual void draw_frame() = 0;

        virtual IPipeline& create_pipeline() const = 0;

    };
}