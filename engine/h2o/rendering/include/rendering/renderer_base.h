#pragma once

#include "core/types.h"
#include "renderer_enums.h"
#include "rendering/renderer.h"

namespace h2o::gfx
{
    class Buffer;
    class VertexArray;

    class Renderer_Base : public IRenderer
    {
    public:

        // Vertex Array
        virtual u32 allocate_vertex_array() = 0;
        virtual void destroy_vertex_array(u32 vertex_array_id) = 0;
        virtual void attach_vertex_buffer(VertexArray& vertex_array, const Buffer& buffer, u32 binding_index, i64 offset, i32 stride) = 0;
        virtual void update_index_buffer(VertexArray& vertex_array, const Buffer& buffer) = 0;
        virtual void setup_attribute(VertexArray& vertex_array, u32 attribute_index, u32 binding_index, AttributeType type, i32 size, u32 relative_offset) = 0;

        // Buffer
        virtual u32 allocate_buffer() = 0;
        virtual void destroy_buffer(u32 buffer_id) = 0;
        virtual void update_buffer_data(Buffer& buffer, const void* data, size_t size) = 0;

    };
}