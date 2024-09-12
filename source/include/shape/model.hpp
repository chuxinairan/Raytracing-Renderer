#pragma once

#include <filesystem>

#include "shape.hpp"
#include "triangle.hpp"

#include "acceleration/bvh.hpp"

class Model : public Shape {
public:
    Model(const std::vector<Triangle>& triangles) 
    {
        auto ts = triangles;
        bvh.build(std::move(ts)); 
    }

    Model(const std::filesystem::path& filename);
    std::optional<HitInfo> intersect(const Ray& ray, float t_min, float t_max) const override;
    
    Bounds getBounds() const override { return bvh.getBounds(); }
private:
    BVH bvh {};
};