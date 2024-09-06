#include "film.hpp"

#include <fstream>

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

  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      const glm::vec3 &pixel = GetPixel(x, y);
      glm::ivec3 i_pixel = glm::clamp(pixel * 255.f, 0.f, 255.f);
      // P3:  file << i_pixel.x << " " << i_pixel.y << " " << i_pixel.z << "\n";
      file << static_cast<uint8_t>(i_pixel.x) << static_cast<uint8_t>(i_pixel.y)
           << static_cast<uint8_t>(i_pixel.z);
    }
  }
}