#pragma once

#include "core/types.h"

namespace h2o::gfx
{
    struct TextureFormat
    {
        v2i size { 0, 0 };
        i32 nb_channels { 0 };

        bool operator==(const TextureFormat& other) const
        {
            return size == other.size && nb_channels == other.nb_channels;
        }
    };

    class ITexture
    {
    public:

        virtual ~ITexture() = default;

        virtual void bind(u32 index) const = 0;

        [[nodiscard]] virtual const TextureFormat& format() const = 0;

    };
}