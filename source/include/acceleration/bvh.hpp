#pragma once

#include <vector>

#include "acceleration/bounds.hpp"

#include "shape/triangle.hpp"
#include "shape/shape.hpp"


struct BVHTreeNode {
    Bounds bounds;
    std::vector<Triangle> triangles;
    BVHTreeNode* children[2];
    size_t depth;
    size_t split_axis;

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

struct alignas(32) BVHNode {
    Bounds bounds;
    union {
        int child1_index;
        int triangles_index;  //总的三角形列表中的索引
    };
    uint16_t triangles_count;  // 该节点拥有的三角形的数量
    uint8_t depth;
    uint8_t split_axis;
};

struct BVHState {
    size_t total_node_count {};
    size_t leaf_node_count {};
    size_t max_leaf_node_triangle_count {};

    void addLeafNode(BVHTreeNode* node) {
        leaf_node_count++;
        max_leaf_node_triangle_count = glm::max(max_leaf_node_triangle_count, node->triangles.size());
    }
};

class BVHTreeNodeAllocator {
public:
    BVHTreeNodeAllocator() : ptr(4096) {};
    BVHTreeNode* allocate() {
        if(ptr == 4096) {
            node_list.push_back(new BVHTreeNode[4096]);
            ptr = 0;
        }
        return &(node_list.back()[ptr++]);
    }
    ~BVHTreeNodeAllocator() {
        for(auto *node : node_list) {
            delete[] node;
        }
        node_list.clear();
        node_list.shrink_to_fit();
    }
private:
    size_t ptr;
    std::vector<BVHTreeNode*> node_list;
};

class BVH : public Shape
{
public:
    void build(std::vector<Triangle> &&triangles);
    std::optional<HitInfo> intersect(Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;
private:
    void recursiveSpilt(BVHTreeNode* node, BVHState& state);
    size_t recursiveFlatten(BVHTreeNode* node);
private:
    BVHTreeNodeAllocator allocator {};
    std::vector<Triangle> ordered_triangles;
    std::vector<BVHNode> nodes;
};