#include "camera/film.hpp"

#include <fstream>

#include "thread/thread_pool.hpp"

#include "utils/rgb.hpp"
#include "utils/profile.hpp"

Film::Film(size_t width, size_t height) : width(width), height(height) {
  pixels.resize(width * height);
}

void Film::save(const std::filesystem::path &filename) {
  // P3(P6)
  // width height
  // 255
  // rgb

  // P3保存ASCII形式RGB数据，而P6直接保存二进制形式，故后者io流的输入会比较快

  // P3:  std::ofstream file(filename);
  std::ofstream file(filename, std::ios::binary);
  file << "P6\n" << width << " " << height << "\n255\n";

  std::vector<uint8_t> buffer(width * height * 3);

  thread_pool.parallelFor(width, height, [&](size_t x, size_t y){
    auto pixel = GetPixel(x, y);
    RGB rgb(pixel.color / static_cast<float>(pixel.sample_count));
    size_t idx = (y * width + x) * 3;
    buffer[idx] = rgb.r;
    buffer[idx + 1] = rgb.g; 
    buffer[idx + 2] = rgb.b; 
  }, false);
  thread_pool.wait();

  file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}