#include "texture_array_opengl.h"

#include "core/log.h"
#include "rendering/texture.h"
#include "texture_opengl.h"

namespace h2o::gfx
{
    TextureArray_OpenGL::TextureArray_OpenGL(size_t array_size)
    {
        m_textures.resize(array_size);
    }

    TextureArray_OpenGL::~TextureArray_OpenGL()
    {
        glDeleteTextures(1, &m_handle);
    }

    void TextureArray_OpenGL::set_texture(i32 index, const std::shared_ptr<ITexture>& texture)
    {
        assert(texture);

        const auto& tex_format = texture->format();

        if (!m_handle)
        {
            glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_handle);
            glTextureStorage3D(m_handle, 1, GL_RGBA8,
                tex_format.size.x, tex_format.size.y, m_textures.size());

            // Set texture parameters
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        if (m_texture_format.has_value() && m_texture_format != tex_format)
        {
            // TODO: Print the path of the texture
            log::warn("Texture was added to a texture "
                      "array with mismatched format.");

            return;
        }

        if (index < 0 || index >= m_textures.size())
        {
            log::warn("Invalid index for texture in texture array");
            return;
        }

        m_texture_format = texture->format();

        auto& tex_gl = m_textures[index];
        tex_gl = std::dynamic_pointer_cast<Texture_OpenGL>(texture);
        assert(tex_gl);
        assert(tex_gl->handle() != 0);

        glCopyImageSubData(
            tex_gl->handle(), GL_TEXTURE_2D,       0, 0, 0, 0,
            m_handle,         GL_TEXTURE_2D_ARRAY, 0, 0, 0, index,
            m_texture_format->size.x, m_texture_format->size.y, 1);
    }

    void TextureArray_OpenGL::bind(u32 index)
    {
        assert(m_handle);

        // TODO: Check if we have enough texture units available
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_handle);
    }
}