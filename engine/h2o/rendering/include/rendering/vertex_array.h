#pragma once

#include "core/types.h"

#include <memory>

namespace h2o::gfx
{
    class IBuffer;

    enum class AttributeType { F32, U32 };

    class IVertexArray
    {
    public:

        virtual ~IVertexArray() = default;

        virtual void attach_vertex_buffer(const std::shared_ptr<IBuffer>& buffer, u32 binding_index, i64 offset, i32 stride) = 0;
        virtual void setup_attribute(u32 attribute_index, u32 binding_index, AttributeType type, i32 size, u32 relative_offset) = 0;

    };
}