#include <glm/glm.hpp>
#include <iostream>

#include "thread_pool.hpp"
#include "film.hpp"

class SimpleTask : public Task
{
  virtual void run() override
  {
    std::cout << "hello world!" << std::endl;
  }
};

int main()
{
  Film film(1920, 1080);
  for (size_t y = 0; y < film.getHeight(); y++)
  {
    for (size_t x = 0; x < film.getWidth(); x++)
    {
      film.setPixel(x, y, {(float)x / film.getWidth(), (float)y / film.getHeight(), 0.2f});
    }
  }

  // time test
  auto last = std::chrono::high_resolution_clock::now();
  film.save("test.ppm");
  auto Time = std::chrono::high_resolution_clock::now() - last;
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(Time);
  std::cout << "Time: " << ms.count() << "ms\n"
            << std::endl;

  // task
  std::cout << "Simple Task:" << std::endl;
  ThreadPool thread_pool(5);
  thread_pool.addTask(new SimpleTask());
  thread_pool.addTask(new SimpleTask());
  thread_pool.addTask(new SimpleTask());
  thread_pool.addTask(new SimpleTask());
  thread_pool.addTask(new SimpleTask());
  thread_pool.addTask(new SimpleTask());
  return 0;
}