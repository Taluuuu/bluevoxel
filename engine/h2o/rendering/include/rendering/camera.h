#pragma once

#include "core/types.h"

namespace h2o::gfx
{
    class Camera
    {
    public:

        explicit Camera(
            f32 fov = 90.0f,
            f32 aspect_ratio = 1.0f,
            v3 position = v3(0.0f),
            v3 rotation = v3(0.0f));

        void update(const v3& position, const v3& rotation);

        [[nodiscard]] m4 calc_view_matrix() const;
        [[nodiscard]] m4 calc_proj_matrix() const;

        [[nodiscard]] const v3& position() const { return m_position; }
        [[nodiscard]] const v3& rotation() const { return m_rotation; }
        [[nodiscard]] const v3& front() const { return m_front; }
        [[nodiscard]] const v3& up() const { return m_up; }

    public:

        static constexpr f32 near = 0.1f;
        static constexpr f32 far = 1000.0f;

        f32 fov = 0.0f;
        f32 aspect_ratio = 0.0f;

    private:

        [[nodiscard]] v3 calc_front() const;
        [[nodiscard]] v3 calc_up() const;

    private:

        v3 m_position{};
        v3 m_rotation{};
        v3 m_front{};
        v3 m_up{};

    };
}