#include "camera/film.hpp"

#include <fstream>

#include "utils/rgb.hpp"

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
      const Pixel& pixel = GetPixel(x, y);
      RGB rgb(pixel.color / static_cast<float>(pixel.sample_count));

      // P3:  file << i_pixel.x << " " << i_pixel.y << " " << i_pixel.z << "\n";
      file << static_cast<uint8_t>(rgb.r) << static_cast<uint8_t>(rgb.g)
           << static_cast<uint8_t>(rgb.b);
    }
  }
}