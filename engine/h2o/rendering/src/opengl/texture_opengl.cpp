#include "texture_opengl.h"

#include <stb_image.h>

namespace h2o::gfx
{
    Texture_OpenGL::~Texture_OpenGL()
    {
        glDeleteTextures(1, &m_handle);
    }

    void Texture_OpenGL::bind(u32 index) const
    {
        glBindTextureUnit(index, m_handle);
    }

    const TextureFormat &Texture_OpenGL::format() const
    {
        return m_texture_format;
    }

    bool Texture_OpenGL::load(const std::string &path)
    {
        int width, height, nb_channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nb_channels, 0);

        if (!data)
        {
            log::warn("Could not load texture at path: {}", path);
            return false;
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);

        glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureStorage2D(m_handle, 1, GL_RGBA8, width, height);
        glTextureSubImage2D(m_handle, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);

        m_texture_format = {{ width, height }, nb_channels};

        return true;
    }
}