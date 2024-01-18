#include "physics/math_helpers.h"

namespace h2o::physics
{
    std::optional<QuadraticResult> find_quadratic_roots(f32 a, f32 b, f32 c)
    {
        const f32 discriminant = b * b - 4.0f * a * c;

        if (discriminant >= 0)
        {
            const f32 r1 = (-b + sqrt(discriminant)) / (2.0f * a);
            const f32 r2 = (-b - sqrt(discriminant)) / (2.0f * a);

            return QuadraticResult{ r1, r2 };
        }

        return std::nullopt;
    }

    // https://antongerdelan.net/opengl/raycasting.html
    v3 screen_to_ray_direction(v2 screen_pos, v2 window_size, const m4& view, const m4& proj)
    {
        const v3 ray_nds{
            (2.0f * screen_pos.x) / window_size.x - 1.0f,
            1.0f - (2.0f * screen_pos.y) / window_size.y, 1.0f };

        const v4 ray_clip{ ray_nds.x, ray_nds.y, -1.0f, 1.0f };

        v4 ray_eye = glm::inverse(proj) * ray_clip;
        ray_eye = v4{ ray_eye.x, ray_eye.y, -1.0f, 0.0f };

        return glm::normalize(glm::inverse(view) * ray_eye);
    }
}