#pragma once

#include "core/types.h"

namespace h2o::gfx
{
    class Camera
    {
    public:

        Camera(f32 fov, f32 aspect_ratio);

        void update(const v3& position, const v3& rotation);

        [[nodiscard]] const m4& proj_view() const { return m_proj_view; }
        [[nodiscard]] const v3& front()     const { return m_front;     }

    public:

        f32 fov;
        f32 aspect_ratio;

        static constexpr f32 near = 0.1f;
        static constexpr f32 far = 1000.0f;

    private:

        m4 m_proj_view{};
        v3 m_front{};

    };
}