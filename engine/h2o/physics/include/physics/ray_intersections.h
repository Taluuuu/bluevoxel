#pragma once

#include "primitives.h"
#include "ray.h"

#include <optional>

namespace h2o::physics
{
    [[nodiscard]] std::optional<f32> intersect_plane(const Ray& ray, const Plane& plane);
    [[nodiscard]] std::optional<f32> intersect_plane_two_directions(const Ray& ray, const Plane& plane);

    [[nodiscard]] std::optional<f32> intersect_disk(const Ray& ray, const Disk& disk);

    [[nodiscard]] std::optional<f32> intersect_cylinder(const Ray& ray, const Cylinder& cylinder);
}