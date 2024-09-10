#pragma once

#include <vector>

#include "acceleration/bounds.hpp"

#include "shape/triangle.hpp"
#include "shape/shape.hpp"


struct BVHTreeNode {
    Bounds bounds;
    std::vector<Triangle> triangles;
    BVHTreeNode* children[2];

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

/*struct BVHNode {
    Bounds bounds;
    size_t child1_index;
    size_t triangles_index;  //总的三角形列表中的索引
    size_t triangles_count;  // 该节点拥有的三角形的数量
};*/

struct alignas(32) BVHNode {
    Bounds bounds;
    union {
        int child1_index;
        int triangles_index;  //总的三角形列表中的索引
    };
    int triangles_count;  // 该节点拥有的三角形的数量
};

class BVH : public Shape
{
public:
    void build(std::vector<Triangle> &&triangles);
    std::optional<HitInfo> intersect(Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;
private:
    void recursiveSpilt(BVHTreeNode* node);
    size_t recursiveFlatten(BVHTreeNode* node);
private:
    std::vector<Triangle> ordered_triangles;
    std::vector<BVHNode> nodes;
};