#pragma once

#include "core/types.h"

namespace h2o::gfx
{
    class ITexture
    {
    public:

        virtual ~ITexture() = default;

        virtual void bind(u32 index) const = 0;

    };
}