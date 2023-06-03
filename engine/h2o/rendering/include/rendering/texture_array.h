#pragma once

#include "core/types.h"

#include <memory>
#include <string>

namespace h2o::gfx
{
    class ITexture;

    class ITextureArray
    {
    public:

        virtual ~ITextureArray() = default;

        virtual void set_texture(i32 index, const std::shared_ptr<ITexture>& texture) = 0;
        virtual void bind(u32 index) = 0;

    };
}