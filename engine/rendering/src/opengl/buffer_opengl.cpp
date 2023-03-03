#include "buffer_opengl.h"

namespace h2o::gfx
{
    Buffer_OpenGL::Buffer_OpenGL()
    {
        glCreateBuffers(1, &m_buffer);
    }

    Buffer_OpenGL::Buffer_OpenGL(Buffer_OpenGL&& other) noexcept
    {
        m_buffer = other.m_buffer;
        other.m_buffer = 0;
    }

    Buffer_OpenGL::~Buffer_OpenGL()
    {
        if (m_buffer)
            glDeleteBuffers(1, &m_buffer);
    }

    void Buffer_OpenGL::update_data(const void* data, size_t size)
    {
        //glNamedBufferData(m_buffer, static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);
        glNamedBufferStorage(m_buffer, static_cast<GLsizeiptr>(size), data, GL_DYNAMIC_STORAGE_BIT);
    }
}