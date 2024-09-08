#pragma once

#include <optional>
#include <glm/glm.hpp>

#include "camera/ray.hpp"

struct Shape {
    virtual std::optional<HitInfo> intersect(Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const = 0;
};