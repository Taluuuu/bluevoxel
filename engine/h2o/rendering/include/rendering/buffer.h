#pragma once

#include "core/types.h"

#include <span>

namespace h2o::gfx
{
    class Renderer_Base;

    class IBuffer
    {
    public:

        virtual ~IBuffer() = default;

        virtual void update_data(const void* data, i32 size) = 0;

    };

    class Buffer
    {
    public:

        explicit Buffer(Renderer_Base& renderer);
        Buffer(Buffer&& other) noexcept;
        Buffer(const Buffer&) = delete;
        ~Buffer();

        void update_data(const void* data, size_t size);
        void destroy();

        [[nodiscard]] bool is_valid() const { return m_id != 0; }
        [[nodiscard]] u32 id() const { return m_id; }

    private:

        Renderer_Base* const m_renderer = nullptr;

        u32 m_id = 0;

    };
}