#include "rendering/texture_array.h"

#include "rendering/texture.h"

namespace h2o::gfx
{
    TextureArray::TextureArray(size_t size)
    {

    }

    void TextureArray::set_texture(u32 index, const std::shared_ptr<Texture>& texture)
    {
        assert(texture);

        const auto& tex_format = texture->format();

        if (!m_format.has_value())
        {
            // Get the texture resolution from the first attached texture
            glTextureStorage3D(m_handle, 1, GL_RGBA8,
                tex_format.size.x, tex_format.size.y, GLsizei(m_textures.size()));

            // Set texture parameters
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            m_format = texture->format();
        }

        if (m_format != tex_format)
        {

        }
    }

    void TextureArray::bind(u32 texture_slot)
    {

    }
}