#pragma once

#include "core/types.h"
#include "texture_format.h"

#include <memory>
#include <optional>
#include <vector>

namespace h2o::gfx
{
    class Texture;

    class TextureArray
    {
    public:

        explicit TextureArray(size_t size);
        TextureArray(const TextureArray&) = delete;
        TextureArray(TextureArray&&) = delete;

        void set_texture(u32 index, const std::shared_ptr<Texture>& texture);
        void bind(u32 texture_slot);

    private:

        u32 m_id = 0;

        std::optional<TextureFormat> m_format{};

        std::vector< std::shared_ptr<Texture> > m_textures{};

    };
}