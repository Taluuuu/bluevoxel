#pragma once

#include "core/types.h"

#include <optional>

namespace h2o::physics
{
    struct Collider_AABB
    {
        v3 position{};
        v3 size{};

        [[nodiscard]] bool intersects(const Collider_AABB& other) const;

        // Returns the movement to apply to a to not collide with b
        [[nodiscard]] static std::optional<v3> resolve(
            const Collider_AABB& a,
            const Collider_AABB& a_prev,
            const Collider_AABB& b);
    };
}
