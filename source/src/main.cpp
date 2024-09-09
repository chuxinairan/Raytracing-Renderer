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
  normal_renderer.render(1, "normal.ppm");
  
  SimpleRTRenderer simple_rt_renderer(camera, scene);
  simple_rt_renderer.render(128, "simple_rt.ppm");

  return 0;
}


// Debug Mode
// Load model: 163ms
// first Parallel for: 773ms
// Parallel for: 485ms
// Save to simple_rt.ppm: 311ms
// Render 1spp normal.ppm: 23993ms
// Render 32spp simple_rt.ppm: 1966102ms
// Render 128spp simple_rt-128spp.ppm: 7726695ms


// Release Mode
// Load model: 29ms
// first Parallel for: 700ms
// Parallel for: 245ms
// Save to simple_rt.ppm: 89ms
// Render 1spp normal.ppm: 1984ms
// Render 32spp simple_rt.ppm: 108775ms


// Change Paralledfor 
// Parallel for: 0ms
// Render 1spp normal.ppm: 1343ms
// Render 128spp simple_rt.ppm: 462954ms

// Change file.save
// Save to simple_rt.ppm: 5ms

// Add bounds
// Load model: 31ms
// Render 1spp normal.ppm: 269ms
// Render 128spp simple_rtspp.ppm: 83884ms

// rapidobj
// Load model: 44ms