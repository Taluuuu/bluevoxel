#pragma once

#include "core/types.h"

namespace h2o::physics
{
    struct Collider_AABB
    {
        v3 position{};
        v3 velocity{};
        v3 size{};

        [[nodiscard]] bool intersects(const Collider_AABB& other) const;

        // Returns true on overlap
        [[nodiscard]] bool resolve(const Collider_AABB& other);
    };
}