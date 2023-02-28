#pragma once

#include "headers_opengl.h"
#include "rendering/buffer.h"

namespace engine
{
    class Buffer_OpenGL : public IBuffer
    {
    public:

        Buffer_OpenGL();
        Buffer_OpenGL(const Buffer_OpenGL&) = delete;
        Buffer_OpenGL(Buffer_OpenGL&& other) noexcept;
        ~Buffer_OpenGL() override;

        // IBuffer interface
        void update_data(const void* data, size_t size) override;

        [[nodiscard]] GLuint handle() const { return m_buffer; }

    private:

        GLuint m_buffer = 0;

    };
}