#include <atomic>
#include <iostream>
#include <random>

#include <glm/glm.hpp>

#include "camera.hpp"
#include "film.hpp"
#include "thread_pool.hpp"

#include "model.hpp"
#include "plane.hpp"
#include "scene.hpp"
#include "sphere.hpp"

#include "frame.hpp"
#include "rgb.hpp"

int main() {
  ThreadPool thread_pool{};
  std::atomic<int> count = 0;

  // Camera
  Film film{192*4, 108*4};
  Camera camera{film, {-3.6, 0, 0}, {0, 0, 0}, 45};

  // Model
  Model model("models/simple_dragon.obj");
  Sphere sphere{{0, 0, 0}, 1};
  Plane plane{{0, 0, 0}, {0, 1, 0}};

  Scene scene{};
  scene.addShape(model, {RGB(202, 159, 117)},
                 {0, 0, 0}, {1, 3, 2});
  scene.addShape(sphere, {{1, 1, 1}, false, RGB(255, 128, 128)}, {0, 0, 2.5});
  scene.addShape(sphere, {{1, 1, 1}, false, RGB(128, 128, 255)}, {0, 0, -2.5});
  scene.addShape(sphere, {{1, 1, 1}, true}, {3, 0.5, -2});
  scene.addShape(plane, {RGB(120, 204, 157)}, {0, -0.5, 0});

  std::mt19937 gen(23451334);
  std::uniform_real_distribution<float> uniform(-1, 1);
  int ssp = 1;

  auto last = std::chrono::high_resolution_clock::now();
  thread_pool.parallelFor(
      film.getWidth(), film.getHeight(), [&](size_t x, size_t y) {
        for (int i = 0; i < ssp; i++) {
          Ray ray = camera.generateRay({x, y}, {abs(uniform(gen)), abs(uniform(gen))});
          glm::vec3 color = {0, 0, 0};
          glm::vec3 beta = {1, 1, 1};

          while (true) {
            auto hit_info = scene.intersect(ray);
            if (hit_info.has_value()) {
              color += beta * hit_info->material->emissive;
              beta *= hit_info->material->albedo;

              ray.origin = hit_info->hit_point;

              Frame frame(hit_info->normal);
              glm::vec3 light_direction;
              if (hit_info->material->is_specular) {
                glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
                light_direction = glm::vec3(-view_direction.x, view_direction.y,
                                            -view_direction.z);
              } else {
                do {
                  light_direction =
                      glm::vec3(uniform(gen), uniform(gen), uniform(gen));
                } while (glm::length(light_direction) > 1);
                if (light_direction.y < 0) {
                  light_direction.y = -light_direction.y;
                }
              }
              ray.direction = frame.worldFromLocal(light_direction);
            } else {
              break;
            }
          }

          film.addSample(x, y, color);
        }

        // processor
        count++;
        if (count % film.getWidth() == 0) {
          std::cout << std::fixed << std::setprecision(2)
                    << static_cast<float>(count) /
                           (film.getWidth() * film.getHeight()) * 100
                    << "%" << std::endl;
        }
      });
  thread_pool.wait();
  auto Time = std::chrono::high_resolution_clock::now() - last;
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(Time);
  std::cout << "Time: " << ms.count() << "ms\n" << std::endl;
  film.save("1920x1080-1采样.ppm");
  return 0;
}