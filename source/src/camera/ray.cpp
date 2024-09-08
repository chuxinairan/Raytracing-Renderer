#include "camera/ray.hpp"

Ray Ray::objectFromWorld(const glm::mat4& object_from_world)
{
    glm::vec3 o = object_from_world * glm::vec4(origin, 1.0f);
    glm::vec3 d = object_from_world * glm::vec4(direction, 0.0f);
    return Ray{ o, d };
}