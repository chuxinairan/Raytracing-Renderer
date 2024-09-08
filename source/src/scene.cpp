#include "scene.hpp"

#include <glm/ext/matrix_transform.hpp>

void Scene::addShape(const Shape &shape, const Material &material,
                     const glm::vec3 &pos, const glm::vec3 &scale,
                     const glm::vec3 &rotation) {
  glm::mat4 world_from_object =
      glm::translate(glm::mat4(1.0f), pos) *
      glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), {1, 0, 0}) *
      glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), {0, 1, 0}) *
      glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), {0, 0, 1}) *
      glm::scale(glm::mat4(1.0f), scale);
  instances.push_back(ShapeInstance{shape, material, world_from_object,
                                    glm::inverse(world_from_object)});
}

std::optional<HitInfo> Scene::intersect(Ray &ray, float t_min,
                                        float t_max) const {
  std::optional<HitInfo> closest_hit_info{};
  const ShapeInstance *closest_inatance = nullptr;
  for (const auto &instance : instances) {
    // 光线世界空间转对象空间
    Ray obj_ray = ray.objectFromWorld(instance.object_from_world);
    auto result = instance.shape.intersect(obj_ray, t_min, t_max);
    if (result.has_value()) {
      t_max = result->t;
      closest_hit_info = result;
      closest_inatance = &instance;
    }
  }

  // hitInfo对象空间转世界空间
  if (closest_inatance) {
    closest_hit_info->hit_point = closest_inatance->world_from_object *
                                  glm::vec4(closest_hit_info->hit_point, 1.0f);
    // 法线的变换比较特殊，变换矩阵是逆矩阵的转置
    closest_hit_info->normal =
        glm::transpose(closest_inatance->object_from_world) *
        glm::vec4(closest_hit_info->normal, 0.0f);
    // 注意归一化之前要将vec4转换成vec3，这里已经进行了自动转换
    closest_hit_info->normal = glm::normalize(closest_hit_info->normal);
    closest_hit_info->material = &closest_inatance->material;
  }
  return closest_hit_info;
}