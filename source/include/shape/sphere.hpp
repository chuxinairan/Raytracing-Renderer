#pragma once

#include "shape.hpp"

struct Sphere : public Shape {
    Sphere(const glm::vec3& center, float radius) : center(center), radius(radius) {}
    std::optional<HitInfo> intersect(Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;
    
    glm::vec3 center;
    float radius;
};