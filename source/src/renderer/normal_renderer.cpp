#include "renderer/normal_renderer.hpp"

glm::vec3 NormalRenderer::renderPixel(const glm::ivec2 &pixelCoord) {
  auto &ray = camera.generateRay(pixelCoord, {rng.uniform(), rng.uniform()});
  auto hit_info = scene.intersect(ray);
  if(hit_info.has_value()){
    return hit_info->normal * 0.5f + 0.5f;
  }
  return {};
}