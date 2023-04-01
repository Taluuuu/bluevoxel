#pragma once

#include "core/types.h"

namespace h2o::gfx
{
    class Camera
    {
    public:

        Camera(
            f32 fov,
            f32 aspect_ratio,
            v3 position = v3(0.0f),
            v3 rotation = v3(0.0f));

        void update(const v3& position, const v3& rotation);

        [[nodiscard]] m4 calc_proj_view() const;
        [[nodiscard]] v3 calc_front() const;

    public:

        f32 fov;
        f32 aspect_ratio;

        static constexpr f32 near = 0.1f;
        static constexpr f32 far = 1000.0f;

    private:

        v3 m_position;
        v3 m_rotation;

    };
}