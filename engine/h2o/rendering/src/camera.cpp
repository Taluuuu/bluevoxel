#include "rendering/camera.h"

namespace h2o::gfx
{
    Camera::Camera(f32 fov, f32 aspect_ratio, v3 position, v3 rotation)
        : fov(fov)
        , aspect_ratio(aspect_ratio)
    {
        update(position, rotation);
    }

    void Camera::update(const v3& position, const v3& rotation)
    {
        m_position = position;
        m_rotation = rotation;
        m_front = calc_front();
        m_up = calc_up();
    }

    m4 Camera::calc_view_matrix() const
    {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    m4 Camera::calc_proj_matrix() const
    {
        return glm::perspective(glm::radians(fov), aspect_ratio, near, far);
    }

    v3 Camera::calc_front() const
    {
        const f32 pitch = m_rotation.x;
        const f32 yaw   = m_rotation.y;

        return glm::normalize(v3{
            glm::cos(yaw) * glm::cos(pitch),
            glm::sin(pitch),
            glm::sin(yaw) * glm::cos(pitch)
        });
    }

    v3 Camera::calc_up() const
    {
        const v3 right = glm::normalize(glm::cross(m_front, { 0.0f, 1.0f, 0.0f }));
        return glm::normalize(glm::cross(right, m_front));
    }
}