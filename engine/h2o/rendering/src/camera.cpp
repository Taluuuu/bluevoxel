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

        m4 rot(1.0f);
        rot = glm::rotate(rot, rotation.y, { 0.0f, 1.0f, 0.0f });
        rot = glm::rotate(rot, rotation.x, { 1.0f, 0.0f, 0.0f });
        rot = glm::rotate(rot, rotation.z, { 0.0f, 0.0f, 1.0f });

        m_front = v3(rot[2]);
        m_up    = v3(rot[1]);
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
        return v3{
            glm::sin(m_rotation.y) * glm::cos(m_rotation.x),
            -glm::sin(m_rotation.x),
            -glm::cos(m_rotation.y) * glm::cos(m_rotation.x)
        };
    }

    v3 Camera::calc_up() const
    {
        const v3 right = glm::normalize(glm::cross(m_front, { 0.0f, 1.0f, 0.0f }));
        return glm::normalize(glm::cross(right, m_front));
    }
}