#pragma once

#include "core/types.h"

#include <span>

namespace engine
{
    class IBuffer
    {
    public:

        virtual ~IBuffer() = default;

        virtual void update_data(const void* data, size_t size) = 0;

    };
}