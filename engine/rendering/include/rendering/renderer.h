#pragma once

#include <memory>

namespace engine
{
    class PipelineFactory;
    class IPipeline;
    class IWindow;

    class IRenderer
    {
    public:

        virtual ~IRenderer() {}

        // TODO: Make this accessible only by the rendering module
        virtual void draw_frame() const = 0;

        virtual IPipeline* create_pipeline(const PipelineFactory& factory) const = 0;

    };
}