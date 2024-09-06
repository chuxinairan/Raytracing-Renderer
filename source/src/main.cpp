#include <glm/glm.hpp>

#include "film.hpp"
#include "thread_pool.hpp"

int main() {
  ThreadPool thread_pool{};
  Film film{1920, 1080};

  thread_pool.parallelFor(1920, 1080, [&](size_t x, size_t y) {
    film.setPixel(x, y, {(float)x / 1920, (float)y / 1080, 0.2});
  });
  thread_pool.wait();
  film.save("test.ppm");
  return 0;
}