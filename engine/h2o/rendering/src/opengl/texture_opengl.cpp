#include "texture_opengl.h"

#include <stb_image.h>

namespace h2o::gfx
{
    std::shared_ptr<Texture_OpenGL> Texture_OpenGL::create(const std::string& path)
    {
        int width, height, nb_channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nb_channels, 0);

        if (data == nullptr)
            return nullptr;

        GLuint handle;
        glCreateTextures(GL_TEXTURE_2D, 1, &handle);

        glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureStorage2D(handle, 1, GL_RGBA8, width, height);
        glTextureSubImage2D(handle, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);

        return std::shared_ptr<Texture_OpenGL>(
            new Texture_OpenGL(handle, { width, height }, nb_channels));
    }

    void Texture_OpenGL::bind(u32 index) const
    {
        glBindTextureUnit(index, m_handle);
    }

    Texture_OpenGL::Texture_OpenGL(GLuint handle, v2i size, i32 nb_channels)
        : m_handle(handle)
        , m_size(size)
        , m_nb_channels(nb_channels)
    {}
}