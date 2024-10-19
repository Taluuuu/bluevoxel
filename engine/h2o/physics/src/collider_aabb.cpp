#include "physics/collider_aabb.h"

#include "core/log.h"

#include <algorithm>
#include <optional>

namespace h2o::physics
{
    bool Collider_AABB::intersects(const Collider_AABB& other) const
    {
        const v3 this_min = position;
        const v3 this_max = position + size;

        const v3 other_min = other.position;
        const v3 other_max = other.position + other.size;

        return
            (this_min.x <= other_max.x) && (this_max.x >= other_min.x) &&
            (this_min.y <= other_max.y) && (this_max.y >= other_min.y) &&
            (this_min.z <= other_max.z) && (this_max.z >= other_min.z);
    }

    std::optional<v3> Collider_AABB::resolve(
        const Collider_AABB& a,
        const Collider_AABB& a_prev,
        const Collider_AABB& b)
    {
        const v3 a_min = a.position;
        const v3 a_max = a_min + a.size;
        const v3 a_old_min = a_prev.position;
        const v3 a_old_max = a_old_min + a_prev.size;

        const v3 b_min = b.position;
        const v3 b_max = b_min + b.size;
        const v3 b_old_min = b_min;
        const v3 b_old_max = b_max;

        // Check for overlap
        if (a_min.x > b_max.x || a_max.x < b_min.x ||
            a_min.y > b_max.y || a_max.y < b_min.y ||
            a_min.z > b_max.z || a_max.z < b_min.z)
            return v3{0.0f}; // No collision

        v3 response{ 0.0f };

        // Bottom
        if (a_max.y > b_min.y && a_old_max.y <= b_old_min.y)
        {
            response += v3(0.0f, b_min.y - a_max.y - 0.001f, 0.0f);
        }

        // Top
        else if (a_min.y < b_max.y && a_old_min.y >= b_old_max.y)
        {
            response += v3(0.0f, b_max.y - a_min.y + 0.001f, 0.0f);
        }

        // X-
        else if (a_max.x > b_min.x && a_old_max.x <= b_old_min.x)
        {
            response += v3(b_min.x - a_max.x - 0.001f, 0.0f, 0.0f);
        }

        // X+
        else if (a_min.x < b_max.x && a_old_min.x >= b_old_max.x)
        {
            response += v3(b_max.x - a_min.x + 0.001f, 0.0f, 0.0f);
        }

        // Z-
        else if (a_max.z > b_min.z && a_old_max.z <= b_old_min.z)
        {
            response += v3(0.0f, 0.0f, b_min.z - a_max.z - 0.001f);
        }

        // Z+
        else if (a_min.z < b_max.z && a_old_min.z >= b_old_max.z)
        {
            response += v3(0.0f, 0.0f, b_max.z - a_min.z + 0.001f);
        }

        return response;
    }
}
