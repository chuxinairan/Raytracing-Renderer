#pragma once

#include <glm/glm.hpp>

#include "shape/material.hpp"

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    glm::vec3 hit(float t) { return origin + direction * t; }
    Ray objectFromWorld(const glm::mat4& object_from_world);
};

struct HitInfo {
    float t;
    glm::vec3 hit_point;
    glm::vec3 normal;
    const Material* material = nullptr;
};