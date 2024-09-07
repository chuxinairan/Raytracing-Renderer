#pragma once

#include "shape.hpp"

struct Triangle : public Shape
{
    Triangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, 
        const glm::vec3& n0, const glm::vec3& n1, const glm::vec3& n2)
        : p0(p0), p1(p1), p2(p2), n0(n0), n1(n1), n2(n2) {}

    Triangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
        : p0(p0), p1(p1), p2(p2) 
        {
            glm::vec3 e0 = p1 - p0;
            glm::vec3 e1 = p2 - p1;
            glm::vec3 normal = glm::normalize(glm::cross(e0, e1));
            n0 = normal;
            n1 = normal;
            n2 = normal;
        }

    virtual std::optional<HitInfo> intersect(Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;

    glm::vec3 p0, p1, p2;
    glm::vec3 n0, n1, n2;
};