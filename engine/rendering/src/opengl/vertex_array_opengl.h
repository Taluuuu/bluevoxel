#pragma once

#include "rendering/vertex_array.h"

#include <glad/gl.h>
#include <vector>

namespace engine
{
    class VertexArray_OpenGL : public IVertexArray
    {
    public:

        VertexArray_OpenGL();
        VertexArray_OpenGL(VertexArray_OpenGL&& other) noexcept;
        VertexArray_OpenGL(const VertexArray_OpenGL&) = delete;
        ~VertexArray_OpenGL() override;

        // IVertexArray interface
        void attach_vertex_buffer(const std::shared_ptr<IBuffer>& buffer, u32 binding_index, i64 offset, i32 stride) override;
        void setup_attribute(u32 attribute_index, u32 binding_index, i32 size, u32 relative_offset) override;

        void bind() const { glBindVertexArray(m_vertex_array); }

    private:

        std::vector< std::shared_ptr<IBuffer> > m_buffers;

        GLuint m_vertex_array = 0;

    };
}