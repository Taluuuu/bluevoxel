#pragma once

#include "core/types.h"

#include <optional>

namespace h2o::physics
{
    struct QuadraticResult
    {
        f32 t1 = 0.0f;
        f32 t2 = 0.0f;
    };

    [[nodiscard]] std::optional<QuadraticResult> find_quadratic_roots(f32 a, f32 b, f32 c);

    [[nodiscard]] v3 screen_to_ray_direction(v2 screen_pos, v2 window_size, const m4& view, const m4& proj);
}