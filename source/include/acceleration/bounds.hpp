#pragma once

#include <glm/glm.hpp>

#include "camera/ray.hpp"

struct Bounds {
    Bounds() :b_min(std::numeric_limits<float>::infinity()), b_max(-std::numeric_limits<float>::infinity()) {}
    Bounds(const glm::vec3& b_min, const glm::vec3& b_max) : b_min(b_min), b_max(b_max) {}

    void expand(const glm::vec3& pos) {
        b_min = glm::min(pos, b_min);
        b_max = glm::max(pos, b_max);
    }
    bool hasIntersection(Ray& ray, float t_min, float t_max) const;

    glm::vec3 diagonal() const { return b_max - b_min; }

    glm::vec3 b_min;
    glm::vec3 b_max;
};