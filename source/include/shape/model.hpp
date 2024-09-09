#pragma once

#include "triangle.hpp"
#include <filesystem>

#include "acceleration/bounds.hpp"

class Model : public Shape {
public:
    Model(const std::vector<Triangle>& triangles) : triangles(triangles) { build(); }

    Model(const std::filesystem::path& filename);

    std::optional<HitInfo> intersect(Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;
    inline size_t GetTriangleCount() {return triangles.size();}
private:
    void build();
private:
    Bounds bound{};
    std::vector<Triangle> triangles;
};