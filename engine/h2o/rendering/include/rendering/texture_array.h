#pragma once

#include "core/types.h"
#include "texture_format.h"

#include <memory>
#include <optional>
#include <vector>

namespace h2o::gfx
{
    class Renderer_Base;
    class Texture;

    class TextureArray
    {
    public:

        TextureArray(Renderer_Base& renderer, u32 size);
        TextureArray(const TextureArray&) = delete;
        TextureArray(TextureArray&&) = delete;
        ~TextureArray();

        void set_texture(u32 index, const std::shared_ptr<Texture>& texture);
        void bind(u32 texture_slot);

        [[nodiscard]] u32 id() const { return m_id; }
        [[nodiscard]] u32 size() const { return m_textures.size(); }

    private:


        std::optional<TextureFormat> m_format{};
        std::vector< std::shared_ptr<Texture> > m_textures{};

        Renderer_Base* const m_renderer = nullptr;

        u32 m_id = 0;

    };
}