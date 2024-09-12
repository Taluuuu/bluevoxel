#include "rendering/vertex_array.h"

#include "core/engine.h"
#include "rendering/renderer_base.h"
#include "rendering/rendering_module.h"

namespace h2o::gfx
{
    VertexArray::VertexArray()
        // TODO: Rework this shit. Need a default constructor, maybe cache a static rendering module ptr ?
        : VertexArray(g_engine->get_module_checked<RenderingModule>().renderer_base())
    {}

    VertexArray::VertexArray(Renderer_Base& renderer)
        : m_renderer(&renderer)
        , m_id(m_renderer->allocate_vertex_array())
    {}

    VertexArray::VertexArray(VertexArray&& other) noexcept
        : m_vertex_buffers(std::move(other.m_vertex_buffers))
        , m_renderer(other.m_renderer)
        , m_id(other.m_id)
    {
        other.m_id = 0;
    }

    VertexArray::~VertexArray()
    {
        m_renderer->destroy_vertex_array(m_id);
    }

    VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
    {
        if (this != &other)
        {
            m_vertex_buffers = std::move(other.m_vertex_buffers);
            m_renderer = other.m_renderer;
            m_id = other.m_id;

            other.m_id = 0;
        }

        return *this;
    }

    void VertexArray::attach_vertex_buffer(const std::shared_ptr<Buffer>& buffer, u32 binding_index, i64 offset, i32 stride)
    {
        assert(buffer != nullptr);
        assert(binding_index < m_vertex_buffers.size());

        m_renderer->attach_vertex_buffer(id(), *buffer, binding_index, offset, stride);
        m_vertex_buffers[binding_index] = buffer;
    }

    void VertexArray::attach_index_buffer(const std::shared_ptr<Buffer>& buffer)
    {
        assert(buffer != nullptr);

        m_renderer->update_index_buffer(id(), *buffer);
        m_index_buffer = buffer;
    }

    void VertexArray::setup_attribute_float(u32 attribute_index, u32 binding_index, AttributeType type, bool normalize, i32 size, u32 relative_offset)
    {
        m_renderer->setup_attribute_float(id(), attribute_index, binding_index, type, normalize, size, relative_offset);
    }

    void VertexArray::setup_attribute_int(u32 attribute_index, u32 binding_index, AttributeType type, i32 size, u32 relative_offset)
    {
        m_renderer->setup_attribute_int(id(), attribute_index, binding_index, type, size, relative_offset);
    }

    const std::shared_ptr<Buffer>& VertexArray::get_vertex_buffer(u32 binding_index) const
    {
        assert(binding_index < m_vertex_buffers.size());
        return m_vertex_buffers[binding_index];
    }
}