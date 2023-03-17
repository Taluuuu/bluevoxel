#pragma once

#include <memory>

namespace h2o
{
    class GameInfo;
    class IWindow;
}

namespace h2o::gfx
{
    class PipelineCreateData;
    class IPipeline;
    class IBuffer;
    class IVertexArray;

    class IRenderer
    {
    public:

        virtual ~IRenderer() = default;

        // Pipeline
        virtual PipelineCreateData            create_pipeline() = 0;
        virtual std::shared_ptr<IPipeline>    compile_pipeline(const PipelineCreateData& create_data) = 0;
        virtual void                          bind_pipeline(const std::shared_ptr<IPipeline>& pipeline) = 0; // Temporary

        virtual std::shared_ptr<IBuffer>      create_buffer() = 0;
        virtual std::shared_ptr<IVertexArray> create_vertex_array() = 0;
        virtual void                          draw(const IVertexArray& vertex_array) = 0;

    protected:

        friend class RenderingModule;

        virtual bool init(IWindow& window, const GameInfo& game_info) = 0;
        virtual void clear() = 0;

    };
}