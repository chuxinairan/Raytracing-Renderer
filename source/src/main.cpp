#include <atomic>
#include <iostream>

#include <glm/glm.hpp>

#include "camera/camera.hpp"
#include "camera/film.hpp"

#include "shape/model.hpp"
#include "shape/plane.hpp"
#include "shape/scene.hpp"
#include "shape/sphere.hpp"

#include "utils/rgb.hpp"
#include "utils/rng.hpp"

#include "renderer/normal_renderer.hpp"
#include "renderer/simple_rt_renderer.hpp"

int main() {
  std::atomic<int> count = 0;

  // Camera
  Film film{192*4, 108*4};
  Camera camera{film, {-3.6f, 0, 0}, {0, 0, 0}, 45};

  // Scene
  Model model("models/simple_dragon.obj");
  Sphere sphere{{0, 0, 0}, 1};
  Plane plane{{0, 0, 0}, {0, 1, 0}};

  Scene scene{};
  scene.addShape(model, {RGB(202, 159, 117)},
                 {0, 0, 0}, {1, 3, 2});
  scene.addShape(sphere, {{1, 1, 1}, false, RGB(255, 128, 128)}, {0, 0, 2.5f});
  scene.addShape(sphere, {{1, 1, 1}, false, RGB(128, 128, 255)}, {0, 0, -2.5f});
  scene.addShape(sphere, {{1, 1, 1}, true}, {3, 0.5f, -2});
  scene.addShape(plane, {RGB(120, 204, 157)}, {0, -0.5f, 0});

  NormalRenderer normal_renderer(camera, scene);
  normal_renderer.render(8, "normal.ppm");
  
  SimpleRTRenderer simple_rt_renderer(camera, scene);
  simple_rt_renderer.render(32, "simple_rt.ppm");

  return 0;
}