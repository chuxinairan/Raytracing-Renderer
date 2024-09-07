#pragma once

#include "triangle.hpp"
#include <filesystem>

class Model : public Shape {
public:
    Model(const std::vector<Triangle>& triangles) : triangles(triangles) {}

    Model(const std::filesystem::path& filename);

    virtual std::optional<HitInfo> intersect(Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;
    inline size_t GetTriangleCount() {return triangles.size();}
private:
    std::vector<Triangle> triangles;
};