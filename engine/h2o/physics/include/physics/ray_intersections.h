#pragma once

#include "primitives.h"
#include "ray.h"

#include <optional>

namespace h2o::physics
{
    template<class PrimitiveType>
    [[nodiscard]] std::optional<f32> intersect(const Ray& ray, const PrimitiveType& primitive);
}