#include "rendering/texture_array.h"

#include "rendering/texture.h"
#include "rendering/renderer_base.h"

namespace h2o::gfx
{
    TextureArray::TextureArray(Renderer_Base& renderer, u32 size)
        : m_renderer(&renderer)
        , m_id(renderer.allocate_texture_array())
    {
        m_textures.resize(size, nullptr);
    }

    TextureArray::~TextureArray()
    {
        m_renderer->destroy_texture_array(m_id);
    }

    void TextureArray::set_texture(u32 index, const std::shared_ptr<Texture>& texture)
    {
        assert(texture);

        const auto& tex_format = texture->format();

        if (!m_format.has_value())
        {
            m_renderer->init_texture_array(*this, texture->format());
            m_format = texture->format();
        }

        if (m_format != tex_format)
        {
            // TODO: Print the path of the texture
            log::warn("Texture was added to a texture array with mismatched format.");
            return;
        }

        if (index >= m_textures.size())
        {
            log::warn("Invalid index for texture in texture array");
            return;
        }

        m_textures[index] = texture;
        m_renderer->attach_texture_to_texture_array(*this, *texture, index);
    }

    void TextureArray::bind(u32 texture_slot)
    {
        m_renderer->bind_texture_array(*this, texture_slot);
    }
}