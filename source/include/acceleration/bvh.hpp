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
            bounds.expand(triangle.getBounds());
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
    uint8_t split_axis;
};

struct BVHState {
    size_t total_node_count {};
    size_t leaf_node_count {};
    size_t max_leaf_node_triangle_count {};
    size_t max_leaf_node_depth {};
    
    void addLeafNode(BVHTreeNode* node) {
        leaf_node_count++;
        max_leaf_node_triangle_count = glm::max(max_leaf_node_triangle_count, node->triangles.size());
        max_leaf_node_depth = glm::max(max_leaf_node_depth, node->depth);
    }
};

class BVHTreeNodeAllocator {
public:
    BVHTreeNodeAllocator() : ptr(4096) {};

    BVHTreeNode* allocate() {
        if(ptr == 4096) {
            nodes_list.push_back(new BVHTreeNode[4096]);
            ptr = 0;
        }
        return &(nodes_list.back()[ptr++]);
    }

    ~BVHTreeNodeAllocator() {
        for(auto *nodes : nodes_list) {
            delete[] nodes;
        }
        nodes_list.clear();
        nodes_list.shrink_to_fit();
    }
private:
    size_t ptr;
    std::vector<BVHTreeNode*> nodes_list;
};

class BVH : public Shape
{
public:
    void build(std::vector<Triangle> &&triangles);
    std::optional<HitInfo> intersect(const Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;

    Bounds getBounds() const override { return nodes[0].bounds; }
private:
    void recursiveSpilt(BVHTreeNode* node, BVHState& state);
    size_t recursiveFlatten(BVHTreeNode* node);
private:
    BVHTreeNodeAllocator allocator {};
    std::vector<Triangle> ordered_triangles;
    std::vector<BVHNode> nodes;
};