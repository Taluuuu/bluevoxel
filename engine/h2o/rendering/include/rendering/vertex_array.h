#pragma once

#include "core/types.h"
#include "renderer_enums.h"

#include <memory>
#include <vector>

namespace h2o::gfx
{
    class Buffer;
    class Renderer_Base;

    class VertexArray
    {
    public:

        explicit VertexArray(Renderer_Base& renderer);
        VertexArray(VertexArray&& other) noexcept;
        VertexArray(const VertexArray&) = delete;
        ~VertexArray();

        void attach_vertex_buffer(const std::shared_ptr<Buffer>& buffer, u32 binding_index, i64 offset, i32 stride);
        void attach_index_buffer(const std::shared_ptr<Buffer>& buffer);

        void setup_attribute(u32 attribute_index, u32 binding_index, AttributeType type, i32 size, u32 relative_offset);

        [[nodiscard]] bool is_valid() const { return m_id != 0; }
        [[nodiscard]] u32 id() const { return m_id; }
        [[nodiscard]] const std::shared_ptr<Buffer>& get_vertex_buffer(u32 binding_index) const;
        [[nodiscard]] const std::shared_ptr<Buffer>& get_index_buffer() const { return m_index_buffer; }

    private:

        std::array<std::shared_ptr<Buffer>, 4> m_vertex_buffers{};
        std::shared_ptr<Buffer> m_index_buffer = nullptr;

        Renderer_Base* const m_renderer = nullptr;

        u32 m_id = 0;

    };
}