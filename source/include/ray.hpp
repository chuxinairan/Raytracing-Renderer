#pragma once

#include <glm/glm.hpp>

struct Ray{
    glm::vec3 origin;
    glm::vec3 direction;

    glm::vec3 hit(float t) { return origin + direction * t; }
};

struct HitInfo{
    float t;
    glm::vec3 hit_point;
    glm::vec3 normal;
};