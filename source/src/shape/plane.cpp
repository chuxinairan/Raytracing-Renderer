#include "shape/plane.hpp"

std::optional<HitInfo> Plane::intersect(Ray& ray, float t_min, float t_max) const
{
    float hit_t = glm::dot(point - ray.origin, normal) / glm::dot(ray.direction, normal);
    if(hit_t <= t_max && hit_t >= t_min) {
        return HitInfo {hit_t, ray.hit(hit_t), normal};
    }
    return {};
}