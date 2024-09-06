#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <filesystem>

class Film
{
public:
    Film(size_t width, size_t height);

    void save(const std::filesystem::path &filename);

    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }

    void setPixel(size_t x, size_t y, const glm::vec3 &color) { pixels[y * width + x] = color; }
    const glm::vec3 &GetPixel(size_t x, size_t y) const { return pixels[y * width + x]; }

private:
    size_t width, height;
    std::vector<glm::vec3> pixels;
};