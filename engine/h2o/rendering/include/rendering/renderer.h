#pragma once

#include "renderer_enums.h"

#include <memory>
#include <string>

namespace h2o
{
    class GameInfo;
    class IWindow;
}

namespace h2o::gfx
{
    class Buffer;
    class IPipeline;
    class ITexture;
    class ITextureArray;
    class Mesh;
    class PipelineCreateData;
    class VertexArray;

    class IRenderer
    {
    public:

        virtual ~IRenderer() = default;

        virtual bool init(IWindow& window, const GameInfo& game_info) = 0;
        virtual void start_frame() = 0;
        virtual void end_frame() = 0;

        virtual void set_scissor(v2i scissor_pos, v2i scissor_size) = 0;

        // Pipeline
        virtual PipelineCreateData             create_pipeline() = 0;
        virtual void                           bind_pipeline(const std::shared_ptr<IPipeline>& pipeline) = 0;

        virtual Buffer                         create_buffer() = 0;
        virtual std::shared_ptr<Buffer>        create_buffer_ptr() = 0;
        virtual VertexArray                    create_vertex_array() = 0;
        virtual std::shared_ptr<VertexArray>   create_vertex_array_ptr() = 0;

        virtual std::shared_ptr<ITexture>      fetch_or_load_texture(const std::string& path) = 0;
        virtual std::shared_ptr<ITextureArray> create_texture_array(size_t array_size) = 0;

        // Drawing
        virtual void                           draw_arrays(const VertexArray& vertex_array, u32 vertex_count) = 0;
        virtual void                           draw_elements(const VertexArray& vertex_array, u32 vertex_count, AttributeType indices_type, u64 byte_offset = 0) = 0;
        virtual void                           draw(const Mesh& mesh) = 0;

    };
}