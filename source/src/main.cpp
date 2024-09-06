#include <glm/glm.hpp>

#include "film.hpp"
#include "camera.hpp"
#include "thread_pool.hpp"

#include "sphere.hpp"

int main() {
  ThreadPool thread_pool{};

  Film film{1920, 1080};
  Camera camera(film, { 0, 0, 1 }, { 0, 0, 0 }, 90);

    Sphere sphere{
        { 0, 0, 0 },
        0.5f
    };

    glm::vec3 light_pos{ 1, 1, 1 };

  thread_pool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y) {
    Ray ray = camera.generateRay({ x, y });
    auto result = sphere.intersect(ray);
    if(result.has_value()){
        auto hit_pos = ray.hit(result.value());
        auto normal = hit_pos - sphere.center;
        auto light = light_pos - hit_pos;
        float cosine = glm::max(0.0f, glm::dot(normal, light));
        film.setPixel(x, y, {cosine, cosine, cosine});
    }
  });
  thread_pool.wait();
  film.save("test.ppm");
  return 0;
}