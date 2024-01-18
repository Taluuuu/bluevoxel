#include "physics/ray_intersections.h"

#include "physics/math_helpers.h"
#include "physics/ray.h"

namespace h2o::physics
{
    std::optional<f32> intersect_plane(const Ray& ray, const Plane& plane)
    {
        static constexpr f32 e = glm::epsilon<f32>();
        const f32 denominator = glm::dot(ray.direction, plane.normal);
        if (denominator < e)
            return std::nullopt;

        const f32 t = glm::dot(plane.point - ray.origin, plane.normal) / denominator;
        return (t >= 0.0f) ? std::optional<f32>{ t } : std::nullopt;
    }

    std::optional<f32> intersect_disk(const Ray& ray, const Disk& disk)
    {
        if (const auto t = intersect_plane(ray, Plane{ disk.center, disk.normal }))
        {
            const v3 plane_intersection = ray.origin + ray.direction * *t;
            const v3 plane_intersection_to_disk_center = disk.center - plane_intersection;
            const f32 distance_sqr = glm::dot(plane_intersection_to_disk_center, plane_intersection_to_disk_center);
            const f32 radius_sqr = disk.radius * disk.radius;
            return (distance_sqr <= radius_sqr) ? t : std::nullopt;
        }

        return std::nullopt;
    }

    // https://hugi.scene.org/online/hugi24/coding%20graphics%20chris%20dragan%20raytracing%20shapes.htm
    // https://mrl.cs.nyu.edu/~dzorin/rendering/lectures/lecture3/lecture3.pdf
    std::optional<f32> intersect_cylinder(const Ray& ray, const Cylinder& cylinder)
    {
        const v3 C = cylinder.p2;
        const v3 D = ray.direction;
        const v3 V = glm::normalize(cylinder.p1 - cylinder.p2);
        const v3 O = ray.origin;
        const v3 X = O - C;

        const f32 d_dot_v = glm::dot(D, V);
        const f32 x_dot_v = glm::dot(X, V);

        const f32 a = glm::dot(D, D) - d_dot_v * d_dot_v;
        const f32 b = 2.0f * (glm::dot(D, X) - glm::dot(D, V) * glm::dot(X, V));
        const f32 c = glm::dot(X, X) - x_dot_v * x_dot_v - cylinder.radius * cylinder.radius;

        std::array<f32, 4> t_values{};
        size_t size = 0;

        if (const auto quadratic_roots = find_quadratic_roots(a, b, c))
        {
            const f32 t1 = quadratic_roots->t1;
            const f32 t2 = quadratic_roots->t2;

            const f32 maxm = glm::distance(cylinder.p1, cylinder.p2);
            const f32 m1 = d_dot_v * t1 + x_dot_v;
            const f32 m2 = d_dot_v * t2 + x_dot_v;

            if (t1 >= 0.0f && m1 >= 0.0f && m1 <= maxm)
                t_values[size++] = t1;

            if (t2 >= 0.0f && m2 >= 0.0f && m2 <= maxm)
                t_values[size++] = t2;
        }

        if (const auto t = intersect_disk(ray, Disk{ cylinder.p1, -V, cylinder.radius }))
            t_values[size++] = *t;

        if (const auto t = intersect_disk(ray, Disk{ cylinder.p2, V, cylinder.radius }))
            t_values[size++] = *t;

        if (size == 0)
            return std::nullopt;

        f32 min_t = FLT_MAX;
        for (size_t i = 0; i < size; i++)
        {
            const f32 t = t_values[i];
            if (t < min_t)
                min_t = t;
        }

        return min_t;
    }
}