#pragma once

#include "shape.hpp"

struct Plane : public Shape {
    Plane(const glm::vec3& point, const glm::vec3& normal) : point(point), normal(glm::normalize(normal)) {}
    std::optional<HitInfo> intersect(Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;

    glm::vec3 point;
    glm::vec3 normal;
};