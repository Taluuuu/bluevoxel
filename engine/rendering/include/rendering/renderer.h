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
        virtual void draw_frame() = 0;

        // Pipeline
        virtual PipelineFactory create_pipeline() = 0;
        virtual IPipeline*      compile_pipeline(const PipelineFactory& factory) = 0;

    };
}