#include <atomic>
#include <iostream>

#include <glm/glm.hpp>

#include "camera.hpp"
#include "film.hpp"
#include "thread_pool.hpp"

#include "model.hpp"
#include "sphere.hpp"

int main() {
  ThreadPool thread_pool{};
  std::atomic<int> count = 0;

  // Camera
  Film film{1920, 1080};
  Camera camera(film, {-1, 0, 0}, {0, 0, 0}, 90);

  // Model
  Model model("models/simple_dragon.obj");
  Sphere sphere{{0, 0, 0}, 0.5f};

  Shape &shape = model;

  // Light
  glm::vec3 light_pos{-1, 2, 1};

  auto last = std::chrono::high_resolution_clock::now();
  thread_pool.parallelFor(
      film.getWidth(), film.getHeight(), [&](size_t x, size_t y) {
        Ray ray = camera.generateRay({x, y});
        auto hit_info = shape.intersect(ray);

        if (hit_info.has_value()) {
          auto normal = hit_info.value().normal;
          auto light_dir =
              glm::normalize(light_pos - hit_info.value().hit_point);
          float cosine = glm::max(0.0f, glm::dot(normal, light_dir));
          film.setPixel(x, y, {cosine, cosine, cosine});
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
  film.save("test.ppm");
  std::cout << "Total Faces: " << model.GetTriangleCount() << std::endl;
  return 0;
}