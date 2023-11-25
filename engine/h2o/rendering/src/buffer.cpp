#include "rendering/buffer.h"

#include "rendering/renderer_base.h"

namespace h2o::gfx
{
    Buffer::Buffer(Renderer_Base& renderer)
        : m_renderer(&renderer)
    {
        m_id = m_renderer->allocate_buffer();
    }

    Buffer::Buffer(Buffer&& other) noexcept
        : m_renderer(other.m_renderer)
        , m_id(other.m_id)
    {
        other.m_id = 0;
    }

    Buffer::~Buffer()
    {
        destroy();
    }

    void Buffer::update_data(const void* data, size_t size)
    {
        m_renderer->update_buffer_data(*this, data, size);
    }

    void Buffer::destroy()
    {
        m_renderer->destroy_buffer(m_id);
    }
}