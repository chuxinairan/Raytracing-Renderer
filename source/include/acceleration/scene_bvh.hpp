﻿#pragma once

#include <vector>

#include "acceleration/bounds.hpp"

#include "shape/shape.hpp"
#include "material/material.hpp"

struct ShapeInstance {
    const Shape& shape;
    const Material* material;
    glm::mat4 world_from_object;
    glm::mat4 object_from_world;

    Bounds bounds {};
    glm::vec3 center {};

    void updateBounds() {
        bounds = {};
        Bounds object_bounds = shape.getBounds();
        for(size_t idx = 0; idx < 8; idx++) {
            // 对象到世界
            glm::vec3 world_corner = world_from_object * glm::vec4(object_bounds.getCorner(idx), 1.0f);
            bounds.expand(world_corner);
        }
        center = (bounds.b_max + bounds.b_min) * 0.5f;
    }
};

struct SceneBVHTreeNode {
    Bounds bounds;
    std::vector<ShapeInstance> instances;
    SceneBVHTreeNode* children[2];
    size_t depth;
    size_t split_axis;

    void updateBounds()
    {
        bounds = {};
        for(const auto& instance : instances)
        {
            bounds.expand(instance.bounds);
        }
    }
};

struct alignas(32) SceneBVHNode {
    Bounds bounds;
    union {
        int child1_index;
        int instances_index;  //总的三角形列表中的索引
    };
    uint16_t instances_count;  // 该节点拥有的三角形的数量
    uint8_t split_axis;
};

struct SceneBVHState {
    size_t total_node_count {};
    size_t leaf_node_count {};
    size_t max_leaf_node_instance_count {};
    size_t max_leaf_node_depth {};
    
    void addLeafNode(SceneBVHTreeNode* node) {
        leaf_node_count++;
        max_leaf_node_instance_count = glm::max(max_leaf_node_instance_count, node->instances.size());
        max_leaf_node_depth = glm::max(max_leaf_node_depth, node->depth);
    }
};

class SceneBVHTreeNodeAllocator {
public:
    SceneBVHTreeNodeAllocator() : ptr(4096) {};
    SceneBVHTreeNode* allocate() {
        if(ptr == 4096) {
            nodes_list.push_back(new SceneBVHTreeNode[4096]);
            ptr = 0;
        }
        return &(nodes_list.back()[ptr++]);
    }
    ~SceneBVHTreeNodeAllocator() {
        for(auto *nodes : nodes_list) {
            delete[] nodes;
        }
        nodes_list.clear();
        nodes_list.shrink_to_fit();
    }
private:
    size_t ptr;
    std::vector<SceneBVHTreeNode*> nodes_list;
};

class SceneBVH : public Shape
{
public:
    void build(std::vector<ShapeInstance> &&instances);
    std::optional<HitInfo> intersect(const Ray& ray, float t_min = 1e-5, float t_max = std::numeric_limits<float>::infinity()) const override;

    Bounds getBounds() const override { return nodes[0].bounds; }
private:
    void recursiveSpilt(SceneBVHTreeNode* node, SceneBVHState& state);
    size_t recursiveFlatten(SceneBVHTreeNode* node);
private:
    SceneBVHTreeNodeAllocator allocator {};
    std::vector<ShapeInstance> ordered_instances;
    std::vector<ShapeInstance> infinity_instances;
    std::vector<SceneBVHNode> nodes;
};