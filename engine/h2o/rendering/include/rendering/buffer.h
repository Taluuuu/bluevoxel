#pragma once

#include "core/types.h"
#include "renderer_enums.h"

#include <functional>

namespace h2o::gfx
{
    class Renderer_Base;

    class Buffer
    {
    public:

        explicit Buffer(Renderer_Base& renderer);
        Buffer(Buffer&& other) noexcept;
        Buffer(const Buffer&) = delete;
        ~Buffer();

        void update_data(const void* data, size_t size, BufferUsage buffer_usage);
        void map_read_write(const std::function<void(void*, size_t)>& function);
        void map_write_only(const std::function<void(void*, size_t)>& function);
        void map_read_only(const std::function<void(const void*, size_t)>& function) const;

        [[nodiscard]] bool is_valid() const { return m_id != 0; }
        [[nodiscard]] u32 id() const { return m_id; }

        void destroy();

    private:

        Renderer_Base* const m_renderer = nullptr;

        u32 m_id = 0;

    };
}