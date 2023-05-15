#pragma once

#include <memory>
#include <string>

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
    class ITexture;

    class IRenderer
    {
    public:

        virtual ~IRenderer() = default;

        virtual bool init(IWindow& window, const GameInfo& game_info) = 0;
        virtual void start_frame() = 0;
        virtual void end_frame() = 0;

        // Pipeline
        virtual PipelineCreateData            create_pipeline() = 0;
        virtual std::shared_ptr<IPipeline>    compile_pipeline(const PipelineCreateData& create_data) = 0;
        virtual void                          bind_pipeline(const std::shared_ptr<IPipeline>& pipeline) = 0; // Temporary

        virtual std::shared_ptr<IBuffer>      create_buffer() = 0;
        virtual std::shared_ptr<IVertexArray> create_vertex_array() = 0;
        virtual std::shared_ptr<ITexture>     create_texture(const std::string& path) = 0;
        virtual void                          draw(const IVertexArray& vertex_array, i32 count) = 0;
    };
}