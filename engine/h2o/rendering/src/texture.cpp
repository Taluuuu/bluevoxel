#include "rendering/texture.h"

#include "core/engine.h"
#include "rendering/renderer_base.h"
#include "rendering/rendering_module.h"

#include <stb_image.h>

namespace h2o::gfx
{
    // NOTE: This constructor is a bit hacky; normally I'd just pass the renderer as a reference, but since
    //       this is a resource the constructor needs to have no parameters.
    Texture::Texture()
        : m_renderer(&g_engine->get_module_checked<RenderingModule>().renderer_base())
        , m_id(m_renderer->allocate_texture())
    {}

    Texture::~Texture()
    {
        destroy();
    }

    void Texture::update_data(const TextureFormat& format, const void* data)
    {
        m_renderer->update_texture_data(id(), format, data);
        m_format = format;
    }

    void Texture::bind(u32 texture_slot)
    {
        m_renderer->bind_texture(id(), texture_slot);
    }

    void Texture::destroy()
    {
        if (m_id != 0)
        {
            m_renderer->destroy_texture(m_id);
            m_id = 0;
        }
    }

    bool Texture::load(const std::string& path)
    {
        int width, height, nb_channels;
        if (unsigned char* data = stbi_load(path.c_str(), &width, &height, &nb_channels, 0))
        {
            update_data({ { width, height }, u32(nb_channels) }, data);
            stbi_image_free(data);

            return true;
        }

        log::warn("Could not load texture at path: {}", path);

        return true;
    }
}
