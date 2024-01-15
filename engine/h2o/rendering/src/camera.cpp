#include "rendering/camera.h"

namespace h2o::gfx
{
    Camera::Camera(f32 fov, f32 aspect_ratio, v3 position, v3 rotation)
        : fov(glm::radians(fov))
        , aspect_ratio(aspect_ratio)
    {
        update(position, rotation);
    }

    void Camera::update(const v3& position, const v3& rotation)
    {
        m_position = position;
        m_rotation = rotation;
        m_front = calc_front();
    }

    m4 Camera::calc_view_matrix()
    {
        const v3 front = calc_front();
        const v3 right = glm::normalize(glm::cross(front, { 0.0f, 1.0f, 0.0f }));
        const v3 up = glm::normalize(glm::cross(right, front));
        return glm::lookAt(m_position, m_position + front, up);
    }

    m4 Camera::calc_proj_matrix()
    {
        return glm::perspective(fov, aspect_ratio, near, far);
    }

    v3 Camera::calc_front() const
    {
        const f32 pitch = m_rotation.x;
        const f32 yaw   = m_rotation.y;

        return glm::normalize(v3{
            glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
            glm::sin(glm::radians(pitch)),
            glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
        });
    }
}