#include "physics/collider_aabb.h"

#include <algorithm>
#include <optional>

namespace h2o::physics
{
    namespace priv
    {
        // DOES NOT WORK, leaving this here for when i get around to swept collisions again
        // // Based on https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/
        // static void sweep_axis(f32 pos1, f32 size1, f32 vel1, f32 pos2, f32 size2, f32& inv_entry, f32& inv_exit, f32& entry, f32& exit)
        // {
        //     // goofy ahh
        //     if (vel1 == 0.0f)
        //         vel1 = ((pos1 > pos2) ? 1.0f : -1.0f) * 0.0000001f;
        //
        //     // Find the distance between the objects on the near and far sides
        //     if (vel1 > 0.0f)
        //     {
        //         inv_entry = pos2 - (pos1 + size1);
        //         inv_exit = (pos2 + size2) - pos1;
        //     }
        //     else
        //     {
        //         inv_entry = (pos2 + size2) - pos1;
        //         inv_exit = pos2 - (pos1 + size1);
        //     }
        //
        //     if (vel1 == 0.0f)
        //     {
        //         entry = -std::numeric_limits<f32>::infinity();
        //         exit = std::numeric_limits<f32>::infinity();
        //     }
        //     else
        //     {
        //         entry = inv_entry / vel1;
        //         exit = inv_exit / vel1;
        //     }
        // }
        //
        // static f32 sweep_aabb(const Collider_AABB& a, const Collider_AABB& b, v3& out_normal)
        // {
        //     v3 inv_entry{}, inv_exit{}, entry{}, exit{};
        //
        //     sweep_axis(a.position.x, a.size.x, a.velocity.x, b.position.x, b.size.x, inv_entry.x, inv_exit.x, entry.x, exit.x);
        //     sweep_axis(a.position.y, a.size.y, a.velocity.y, b.position.y, b.size.y, inv_entry.y, inv_exit.y, entry.y, exit.y);
        //     sweep_axis(a.position.z, a.size.z, a.velocity.z, b.position.z, b.size.z, inv_entry.z, inv_exit.z, entry.z, exit.z);
        //
        //     // Find the earliest/latest times of collision
        //     const f32 entry_time = std::max({ entry.x, entry.y, entry.z });
        //     const f32 exit_time = std::min({ exit.x, exit.y, exit.z });
        //
        //     // If there was no collision
        //     if ((entry_time > exit_time) ||
        //         (entry.x < 0.0f && entry.y < 0.0f && entry.z < 0.0f) ||
        //         (entry.x > 1.0f || entry.y > 1.0f || entry.z > 1.0f))
        //     {
        //         out_normal = v3{0.0f};
        //         return 1.0f;
        //     }
        //
        //     // || entry.x < 0.0f && entry.y < 0.0f && entry.z < 0.0f || entry.x > 1.0f || entry.y > 1.0f || entry.z > 1.0f
        //
        //     // There was a collision
        //     // Calculate normal of collided surface
        //     if (entry.x > entry.y && entry.x > entry.z)
        //     {
        //         out_normal = (inv_entry.x < 0.0f) ?
        //             v3{ 1.0f, 0.0f, 0.0f } :
        //             v3{-1.0f, 0.0f, 0.0f };
        //     }
        //     else if (entry.y > entry.x && entry.y > entry.z)
        //     {
        //         out_normal = (inv_entry.y < 0.0f) ?
        //             v3{ 0.0f, 1.0f, 0.0f } :
        //             v3{ 0.0f,-1.0f, 0.0f };
        //     }
        //     else
        //     {
        //         out_normal = (inv_entry.z < 0.0f) ?
        //             v3{ 0.0f, 0.0f, 1.0f } :
        //             v3{ 0.0f, 0.0f,-1.0f };
        //     }
        //
        //     // Return the time of collision
        //     return entry_time;
        // }

        std::optional<v3> resolve_discrete_aabb_aabb_collision(const Collider_AABB& c1, const Collider_AABB& c2)
        {
            const v3 c1_min = c1.position + c1.velocity;
            const v3 c1_max = c1_min + c1.size;
            const v3 c1_old_min = c1.position;
            const v3 c1_old_max = c1_old_min + c1.size;

            const v3 c2_min = c2.position + c2.velocity;
            const v3 c2_max = c2_min + c2.size;
            const v3 c2_old_min = c2.position;
            const v3 c2_old_max = c2_old_min + c2.size;

            // Check for overlap
            if (c1_min.x > c2_max.x || c1_max.x < c2_min.x ||
                c1_min.y > c2_max.y || c1_max.y < c2_min.y ||
                c1_min.z > c2_max.z || c1_max.z < c2_min.z)
                return v3{0.0f}; // No collision

            v3 response{ 0.0f };

            // Y-
            if (c1_max.y > c2_min.y && c1_old_max.y <= c2_old_min.y)
                response += v3(0.0f, c2_min.y - c1_max.y - 0.001f, 0.0f);

            // Y+
            else if (c1_min.y < c2_max.y && c1_old_min.y >= c2_old_max.y)
                response += v3(0.0f, c2_max.y - c1_min.y + 0.001f, 0.0f);

            // X-
            else if (c1_max.x > c2_min.x && c1_old_max.x <= c2_old_min.x)
                response += v3(c2_min.x - c1_max.x - 0.001f, 0.0f, 0.0f);

            // X+
            else if (c1_min.x < c2_max.x && c1_old_min.x >= c2_old_max.x)
                response += v3(c2_max.x - c1_min.x + 0.001f, 0.0f, 0.0f);

            // Z-
            else if (c1_max.z > c2_min.z && c1_old_max.z <= c2_old_min.z)
                response += v3(0.0f, 0.0f, c2_min.z - c1_max.z - 0.001f);

            // Z+
            else if (c1_min.z < c2_max.z && c1_old_min.z >= c2_old_max.z)
                response += v3(0.0f, 0.0f, c2_max.z - c1_min.z + 0.001f);

            return response;
        }
    }

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

    bool Collider_AABB::resolve(const Collider_AABB& other)
    {
        v3 normal{};

        const auto resolution = priv::resolve_discrete_aabb_aabb_collision(*this, other);
        if (!resolution)
            return false;

        position += velocity + *resolution;

        // const f32 collision_time = priv::sweep_aabb(*this, other, normal);
        // if (collision_time >= 1.0f)
        //     return false;
        //
        // position += velocity * collision_time;
        //
        // const f32 remaining_time = 1.0f - collision_time;
        //
        // // push
        // float magnitude = sqrt((velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z)) * remaining_time;
        // float dotprod = glm::dot(velocity, normal);
        //
        // if (dotprod > 0.0f) dotprod = 1.0f;
        // else if (dotprod < 0.0f) dotprod = -1.0f;
        //
        // velocity = dotprod * normal * magnitude;

        // // Slide
        // const f32 dotprod = glm::dot(velocity, normal) * remaining_time;
        // velocity.x = velocity.x - 2.0f * dotprod * normal.x;
        // velocity.y = velocity.y - 2.0f * dotprod * normal.y;
        // velocity.z = velocity.z - 2.0f * dotprod * normal.z;

        return true;
    }
}
