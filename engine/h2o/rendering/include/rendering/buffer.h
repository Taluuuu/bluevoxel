#pragma once

#include "core/types.h"

#include <span>

namespace h2o::gfx
{
    class IBuffer
    {
    public:

        virtual ~IBuffer() = default;

        virtual void update_data(const void* data, i32 size) = 0;

    };
}