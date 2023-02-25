#include "vertex_array_opengl.h"

#include "buffer_opengl.h"

#include <cassert>

namespace engine
{
    VertexArray_OpenGL::VertexArray_OpenGL()
    {
        glCreateVertexArrays(1, &m_vertex_array);
    }

    VertexArray_OpenGL::VertexArray_OpenGL(VertexArray_OpenGL&& other) noexcept
    {
        m_vertex_array = other.m_vertex_array;
        other.m_vertex_array = 0;
    }

    VertexArray_OpenGL::~VertexArray_OpenGL()
    {
        if (m_vertex_array)
            glDeleteVertexArrays(1, &m_vertex_array);
    }

    void VertexArray_OpenGL::attach_vertex_buffer(
        const std::shared_ptr<IBuffer>& buffer,
        u32 binding_index,
        i64 offset,
        i32 stride)
    {
        auto buffer_gl = std::dynamic_pointer_cast<Buffer_OpenGL>(buffer);
        assert(buffer_gl != nullptr);

        glVertexArrayVertexBuffer(m_vertex_array, binding_index, buffer_gl->handle(), offset, stride);
    }

    void VertexArray_OpenGL::setup_attribute(
        u32 attribute_index,
        u32 binding_index,
        i32 size,
        u32 relative_offset)
    {
        glEnableVertexArrayAttrib(m_vertex_array, attribute_index);
        glVertexArrayAttribFormat(m_vertex_array, attribute_index, size, GL_FLOAT, GL_FALSE, relative_offset);
        glVertexArrayAttribBinding(m_vertex_array, attribute_index, binding_index);
    }
}