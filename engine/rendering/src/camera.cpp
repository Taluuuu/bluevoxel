#include "rendering/camera.h"

namespace h2o::gfx
{
    Camera::Camera(f32 fov, f32 aspect_ratio)
        : fov(fov)
        , aspect_ratio(aspect_ratio)
    { }

    void Camera::update(const v3& position, const v3& rotation)
    {
        f32 pitch = rotation.x;
        f32 yaw = rotation.y;

        m_front = glm::normalize(v3(
            glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
            glm::sin(glm::radians(pitch)),
            glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
        ));

        v3 right = glm::normalize(glm::cross(m_front, { 0.0f, 1.0f, 0.0f }));
        v3 up = glm::normalize(glm::cross(right, m_front));

        m4 proj = glm::perspective(fov, aspect_ratio, near, far);
        m4 view = glm::lookAt(position, position + m_front, up);

        m_proj_view = proj * view;
    }
}