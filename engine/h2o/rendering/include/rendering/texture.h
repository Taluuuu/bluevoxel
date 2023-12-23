#pragma once

#include "core/resources.h"
#include "core/types.h"
#include "texture_format.h"

namespace h2o::gfx
{
    class Renderer_Base;

    class Texture : public IResource
    {
    public:

        Texture();
        Texture(const Texture&) = delete;
        Texture(Texture&&) = delete;
        ~Texture() override;

        void update_data(const TextureFormat& format, const void* data);
        void bind(u32 texture_slot);
        void destroy();

        [[nodiscard]] const TextureFormat& format() const { return m_format; };
        [[nodiscard]] u32 id() const { return m_id; }

        // IResource interface
        bool load(const std::string& path) override;

    private:

        Renderer_Base* const m_renderer = nullptr;

        u32 m_id = 0;

        TextureFormat m_format{};

    };
}