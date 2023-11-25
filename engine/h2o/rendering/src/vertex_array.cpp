#include "rendering/vertex_array.h"

#include "rendering/renderer_base.h"

namespace h2o::gfx
{
    VertexArray::VertexArray(Renderer_Base& renderer)
        : m_renderer(&renderer)
    {
        m_id = m_renderer->allocate_vertex_array();
    }

    VertexArray::VertexArray(VertexArray&& other) noexcept
        : m_renderer(other.m_renderer)
        , m_id(other.m_id)
    {
        other.m_id = 0;
    }

    VertexArray::~VertexArray()
    {
        m_renderer->destroy_vertex_array(m_id);
    }

    void VertexArray::attach_vertex_buffer(const std::shared_ptr<Buffer>& buffer, u32 binding_index, i64 offset, i32 stride)
    {
        assert(buffer != nullptr);
        assert(binding_index < m_vertex_buffers.size());

        m_renderer->attach_vertex_buffer(*this, *buffer, binding_index, offset, stride);
        m_vertex_buffers[binding_index] = buffer;
    }

    void VertexArray::attach_index_buffer(const std::shared_ptr<Buffer>& buffer)
    {
        assert(buffer != nullptr);

        m_renderer->update_index_buffer(*this, *buffer);
        m_index_buffer = buffer;
    }

    void VertexArray::setup_attribute(u32 attribute_index, u32 binding_index, AttributeType type, i32 size, u32 relative_offset)
    {
        m_renderer->setup_attribute(*this, attribute_index, binding_index, type, size, relative_offset);
    }

    const std::shared_ptr<Buffer>& VertexArray::get_vertex_buffer(u32 binding_index) const
    {
        assert(binding_index < m_vertex_buffers.size());
        return m_vertex_buffers[binding_index];
    }
}