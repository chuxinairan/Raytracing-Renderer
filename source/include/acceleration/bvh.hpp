#pragma once

#include <vector>

#include "acceleration/bounds.hpp"

#include "shape/triangle.hpp"
#include "shape/shape.hpp"

struct BVHNode {
    Bounds bounds;
    std::vector<Triangle> triangles;
    BVHNode* children[2];

    void updateBounds()
    {
        bounds = {};
        for(const auto& triangle : triangles)
        {
            bounds.expand(triangle.p0);
            bounds.expand(triangle.p1);
            bounds.expand(triangle.p2);
        }
    }
};

class BVH : public Shape
{
public:
    void build(std::vector<Triangle> &&triangles);
    std::optional<HitInfo> intersect(Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;
private:
    void recursiveSpilt(BVHNode* node);
    // t_max要使用引用，因为要更新传入递归的的值
    void recursiveIntersect(BVHNode* node, Ray& ray, float t_min, float& t_max, std::optional<HitInfo>& closest_hit) const;
private:
    BVHNode* root;
};