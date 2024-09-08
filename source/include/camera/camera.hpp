#pragma once
#include <glm/glm.hpp>

#include "film.hpp"
#include "ray.hpp"

class Camera
{
public:
  Camera(Film& film, const glm::vec3& pos, const glm::vec3& viewpoint, float fovy);

  // 传入屏幕空间下的一个像素坐标，以及一定的偏移然后返回光线
  Ray generateRay(const glm::ivec2& pixel_coord, const glm::vec2& offset = {0.5f, 0.5f}) const;

  Film getFilm() {return film;}
private:
  Film &film;
  glm::vec3 pos;

  glm::mat4 camera_from_clip;
  glm::mat4 world_from_camera;
};