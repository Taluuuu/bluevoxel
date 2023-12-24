#pragma once

namespace h2o::gfx
{
    struct TextureFormat
    {
        v2u size { 0, 0 };
        u32 nb_channels = 0;

        bool operator==(const TextureFormat& rhs) const
        {
            return size == rhs.size && nb_channels == rhs.nb_channels;
        }
    };
}