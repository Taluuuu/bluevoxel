#pragma once

#include "core/types.h"
#include "renderer_enums.h"
#include "rendering/renderer.h"

namespace h2o::gfx
{
    class VertexArray;
    class Buffer;
    class Texture;
    struct TextureFormat;

    // This class is meant to house all backend-dependant code and provide lower level access to the graphics api
    class Renderer_Base : public IRenderer
    {
    public:

        // Pipeline
        virtual std::shared_ptr<IPipeline> compile_pipeline(const PipelineCreateData& create_data) = 0;

        // Vertex Array
        virtual u32 allocate_vertex_array() = 0;
        virtual void destroy_vertex_array(u32 vertex_array_id) = 0;
        virtual void attach_vertex_buffer(VertexArray& vertex_array, const Buffer& buffer, u32 binding_index, i64 offset, i32 stride) = 0;
        virtual void update_index_buffer(VertexArray& vertex_array, const Buffer& buffer) = 0;
        virtual void setup_attribute(VertexArray& vertex_array, u32 attribute_index, u32 binding_index, AttributeType type, i32 size, u32 relative_offset) = 0;

        // Buffer
        virtual u32 allocate_buffer() = 0;
        virtual void destroy_buffer(u32 buffer_id) = 0;
        virtual void update_buffer_data(Buffer& buffer, const void* data, size_t size, BufferUsage buffer_usage) = 0;

        // Texture
        virtual u32 allocate_texture() = 0;
        virtual void destroy_texture(u32 texture_id) = 0;
        virtual void bind_texture(Texture& texture, u32 texture_slot) = 0;
        virtual void update_texture_data(Texture& texture, const TextureFormat& format, const void* data) = 0;

    };
}