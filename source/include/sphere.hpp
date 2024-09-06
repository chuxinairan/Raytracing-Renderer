#include <optional>
#include <glm/glm.hpp>

#include "ray.hpp"

struct Sphere
{
    glm::vec3 center;
    float radius;

    std::optional<float> intersect(const Ray& ray) const;
};