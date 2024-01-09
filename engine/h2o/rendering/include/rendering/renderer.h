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
    class Mesh;
    class PipelineCreateData;
    class Texture;
    class TextureArray;
    class VertexArray;

    class IRenderer
    {
    public:

        virtual ~IRenderer() = default;

        virtual void set_clear_color(const v4& color) = 0;
        virtual void set_scissor(v2i scissor_pos, v2i scissor_size) = 0;

        // Pipeline
        virtual PipelineCreateData            create_pipeline() = 0;
        virtual void                          bind_pipeline(const std::shared_ptr<IPipeline>& pipeline) = 0;

        // Buffer
        virtual Buffer                        create_buffer() = 0;
        virtual std::shared_ptr<Buffer>       create_buffer_ptr() = 0;

        // Vertex Array
        virtual VertexArray                   create_vertex_array() = 0;
        virtual std::shared_ptr<VertexArray>  create_vertex_array_ptr() = 0;

        // Texture
        virtual Texture                       create_texture() = 0;
        virtual std::shared_ptr<Texture>      create_texture_ptr() = 0;
        virtual TextureArray                  create_texture_array(u32 array_size) = 0;
        virtual std::shared_ptr<TextureArray> create_texture_array_ptr(u32 array_size) = 0;

        // Drawing
        virtual void                          draw_arrays(const VertexArray& vertex_array, u32 vertex_count, DrawMode draw_mode) = 0;
        virtual void                          draw_elements(const VertexArray& vertex_array, u32 vertex_count, AttributeType indices_type, u64 byte_offset = 0) = 0;
        virtual void                          draw(const Mesh& mesh) = 0;

        // Debug
        // TODO: Make the renderer know the camera and set itself the proj view matrix in the shader
        virtual void                          set_proj_view_matrix(const m4& proj_view) = 0;
        virtual void                          draw_debug_line(const v3& origin, const v3& end, const v4& color) = 0;

    };
}