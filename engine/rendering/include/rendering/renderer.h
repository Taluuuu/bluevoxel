#pragma once

#include <memory>

namespace engine
{
    class GameInfo;
    class PipelineCreateData;
    class IPipeline;
    class IWindow;

    class IRenderer
    {
    public:

        virtual ~IRenderer() = default;

        // Pipeline
        virtual PipelineCreateData         create_pipeline() = 0;
        virtual std::shared_ptr<IPipeline> compile_pipeline(const PipelineCreateData& create_data) = 0;
        virtual void                       bind_pipeline(const std::shared_ptr<IPipeline>& pipeline) = 0; // Temporary

    protected:

        friend class RenderingModule;

        virtual bool init(const GameInfo& game_info, IWindow& window) = 0;
        virtual void draw_frame() = 0;

    };
}