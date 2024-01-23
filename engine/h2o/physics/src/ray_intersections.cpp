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

    std::optional<f32> intersect_plane_two_directions(const Ray& ray, const Plane& plane)
    {
        if (const auto t = intersect_plane(ray, plane))
            return t;

        const Plane inverted_plane{ plane.point, -plane.normal };
        if (const auto t = intersect_plane(ray, inverted_plane))
            return t;

        return std::nullopt;
    }

    std::optional<f32> intersect_disk(const Ray& ray, const Disk& disk)
    {
        if (const auto t = intersect_plane(ray, Plane{ disk.center, disk.normal }))
        {
            const v3 plane_intersection = ray.point_at(*t);
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

    // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    std::optional<f32> intersect_triangle(const Ray& ray, const Triangle& triangle)
    {
        constexpr f32 epsilon = glm::epsilon<f32>();

        const v3 edge1 = triangle.p2 - triangle.p1;
        const v3 edge2 = triangle.p3 - triangle.p1;
        const v3 ray_cross_e2 = glm::cross(ray.direction, edge2);
        const f32 det = dot(edge1, ray_cross_e2);

        if (det > -epsilon && det < epsilon)
            return std::nullopt;    // This ray is parallel to this triangle.

        const f32 inv_det = 1.0f / det;
        const v3 s = ray.origin - triangle.p1;
        const f32 u = inv_det * dot(s, ray_cross_e2);

        if (u < 0.0f || u > 1.0f)
            return std::nullopt;

        const v3 s_cross_e1 = glm::cross(s, edge1);
        const f32 v = inv_det * glm::dot(ray.direction, s_cross_e1);

        if (v < 0 || u + v > 1)
            return std::nullopt;

        // At this stage we can compute t to find out where the intersection point is on the line.
        return inv_det * dot(edge2, s_cross_e1);
    }

    // https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
    std::optional<f32> intersect_sphere(const Ray& ray, const Sphere& sphere)
    {
        const v3 L = ray.origin - sphere.center;
        const f32 a = glm::dot(ray.direction, ray.direction);
        const f32 b = 2.0f * glm::dot(ray.direction, L);
        const f32 c = glm::dot(L, L) - sphere.radius * sphere.radius;

        if (const auto roots = find_quadratic_roots(a, b, c))
        {
            const f32 t1 = roots->t1;
            const f32 t2 = roots->t2;

            if (t1 < t2 && t1 > 0.0f)
                return t1;

            if (t2 > 0.0f)
                return t2;
        }

        return std::nullopt;
    }
}