#pragma once

#include "triangle.hpp"
#include <filesystem>

#include "acceleration/bvh.hpp"

class Model : public Shape {
public:
    Model(const std::vector<Triangle>& triangles) 
    {
        auto ts = triangles;
        bvh.build(std::move(ts)); 
    }

    Model(const std::filesystem::path& filename);

    std::optional<HitInfo> intersect(Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;
private:
    BVH bvh;
};