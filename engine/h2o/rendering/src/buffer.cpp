#include "rendering/buffer.h"

#include "rendering/renderer_base.h"

namespace h2o::gfx
{
    Buffer::Buffer(Renderer_Base& renderer)
        : m_renderer(&renderer)
        , m_id(renderer.allocate_buffer())
    {}

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

    void Buffer::update_data(const void* data, size_t size, BufferUsage buffer_usage)
    {
        m_renderer->update_buffer_data(id(), data, size, buffer_usage);
    }

    void Buffer::map_read_only(const std::function<void(const void*)>& function) const
    {
        const void* data = m_renderer->map_buffer_read_only(id());
        function(data);
        m_renderer->unmap_buffer(id());
    }

    void Buffer::map_write_only(const std::function<void(void*)>& function)
    {
        void* data = m_renderer->map_buffer_write_only(id());
        function(data);
        m_renderer->unmap_buffer(id());
    }

    void Buffer::map_read_write(const std::function<void(void*)>& function)
    {
        void* data = m_renderer->map_buffer_read_write(id());
        function(data);
        m_renderer->unmap_buffer(id());
    }

    void Buffer::destroy()
    {
        if (m_id != 0)
        {
            m_renderer->destroy_buffer(m_id);
            m_id = 0;
        }
    }
}