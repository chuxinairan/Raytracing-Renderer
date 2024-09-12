﻿#include "acceleration/bvh.hpp"

#include <array>
#include <iostream>

#include "utils/debug_macro.hpp"

void BVH::build(std::vector<Triangle> &&triangles)
{
    auto* root = allocator.allocate();

    root->triangles = std::move(triangles);
    root->updateBounds();
    root->depth = 1;
    BVHState state {};
    size_t triangle_count = root->triangles.size();
    recursiveSpilt(root, state);

    std::cout << "Total_Node_Count: " << state.total_node_count << std::endl;
    std::cout << "Leaf_Node_Count: " << state.leaf_node_count << std::endl;
    std::cout << "Total_Triangle_Count: " << triangle_count << std::endl;
    std::cout << "Mean_leaf_node_triangle_count: " << static_cast<float>(triangle_count ) / state.leaf_node_count << std::endl;
    std::cout << "Max_leaf_node_triangle_count: " << state.max_leaf_node_triangle_count << std::endl;
    std::cout << "Max_leaf_node_depth: " << state.max_leaf_node_depth << std::endl;

    nodes.reserve(state.total_node_count);
    ordered_triangles.reserve(triangle_count);
    recursiveFlatten(root);
}

void BVH::recursiveSpilt(BVHTreeNode* node, BVHState& state)
{
    state.total_node_count++;
    if(node->triangles.size() == 1 || node->depth > 32) {
        state.addLeafNode(node);
        return;
    }

    auto diag = node->bounds.diagonal();
    constexpr size_t bucket_count = 12;
    size_t min_split_index = 0;
    float min_cost = std::numeric_limits<float>::infinity();
    Bounds min_child0_bounds, min_child1_bounds;
    size_t min_child0_triangle_count, min_child1_triangle_count;
    std::vector<size_t> triangle_indices_per_bucket[3][bucket_count] = {};
    for(size_t axis = 0; axis < 3; axis++) {
        size_t triangle_count_per_bucket[bucket_count] = {};
        Bounds bucket_bounds[bucket_count] = {};
        size_t triangle_idx = 0;
        for(const auto& triangle : node->triangles) {
            float triangle_center = (triangle.p0[axis] + triangle.p1[axis] + triangle.p2[axis]) / 3.0f;
            size_t bucket_idx = glm::clamp<size_t>(glm::floor((triangle_center - node->bounds.b_min[axis]) / diag[axis] * bucket_count), 0, bucket_count-1);
            assert(bucket_idx >= 0 && bucket_idx < bucket_count);
            bucket_bounds[bucket_idx].expand(triangle.p0);
            bucket_bounds[bucket_idx].expand(triangle.p1);
            bucket_bounds[bucket_idx].expand(triangle.p2);
            triangle_count_per_bucket[bucket_idx]++;
            triangle_indices_per_bucket[axis][bucket_idx].push_back(triangle_idx);
            triangle_idx++;
        }

        Bounds left_bounds = bucket_bounds[0];
        size_t left_triangle_count = triangle_count_per_bucket[0];
        for(size_t i = 1; i < bucket_count; i++) {
            Bounds right_bounds{};
            size_t right_triangle_count = 0;
            for(size_t j = bucket_count-1; j >= i; j--) {
                right_bounds.expand(bucket_bounds[j]);
                right_triangle_count += triangle_count_per_bucket[j];
            }

            if(right_triangle_count == 0) {
                break;
            }
            if(left_triangle_count != 0) {
                float cost = left_triangle_count * left_bounds.area() + right_triangle_count * right_bounds.area();
                if(cost < min_cost) {
                    min_cost = cost;
                    min_split_index = i;
                    node->split_axis = axis;
                    min_child0_bounds = left_bounds;
                    min_child1_bounds = right_bounds;
                    min_child0_triangle_count = left_triangle_count;
                    min_child1_triangle_count = right_triangle_count;
                }
            }
            left_bounds.expand(bucket_bounds[i]);
            left_triangle_count += triangle_count_per_bucket[i];
        }
    }

    if(min_split_index == 0) {
        state.addLeafNode(node);
        return;
    }

    BVHTreeNode* child0 = allocator.allocate();
    BVHTreeNode* child1 = allocator.allocate();

    node->children[0] = child0;
    node->children[1] = child1;
    node->children[0]->depth = node->depth + 1;
    node->children[1]->depth = node->depth + 1;

    child0->triangles.reserve(min_child0_triangle_count);
    for(size_t i = 0; i < min_split_index; i++) {
        for(size_t idx : triangle_indices_per_bucket[node->split_axis][i]) {
            child0->triangles.push_back(node->triangles[idx]);
        }
    }
    child1->triangles.reserve(min_child1_triangle_count);
    for(size_t i = min_split_index; i < bucket_count; i++) {
        for(size_t idx : triangle_indices_per_bucket[node->split_axis][i]) {
            child1->triangles.push_back(node->triangles[idx]);
        }
    }

    node->triangles.clear();
    node->triangles.shrink_to_fit();

    child0->bounds = min_child0_bounds;
    child1->bounds = min_child1_bounds;

    recursiveSpilt(child0, state);
    recursiveSpilt(child1, state);
}


size_t BVH::recursiveFlatten(BVHTreeNode* node)
{
    BVHNode linear_node{
        node->bounds,
        0,
        static_cast<uint16_t>(node->triangles.size()),
        static_cast<uint8_t>(node->split_axis)
    };
    size_t idx = nodes.size();
    nodes.push_back(linear_node);
    if(nodes[idx].triangles_count == 0) {
        recursiveFlatten(node->children[0]);
        nodes[idx].child1_index = recursiveFlatten(node->children[1]);
    } else {
        nodes[idx].triangles_index = ordered_triangles.size();
        for(const auto& triangle : node->triangles) {
            ordered_triangles.push_back(std::move(triangle));
        }
    }
    return idx;
}

std::optional<HitInfo> BVH::intersect(const Ray& ray, float t_min, float t_max) const 
{
    std::optional<HitInfo> closest_hitinfo;
    std::array<int, 32> stack;
    auto ptr = stack.begin();

    glm::bvec3 dir_is_neg {
        ray.direction.x < 0,
        ray.direction.y < 0,
        ray.direction.z < 0
    };
    glm::vec3 inv_direction = 1.0f / ray.direction;

    DEBUG_LINE(size_t bounds_test_count = 0, triangle_test_count = 0)

    size_t current_node_index = 0;
    while(true) {
        auto& node = nodes[current_node_index];
        DEBUG_LINE(bounds_test_count++)
        if(!node.bounds.hasIntersection(ray, inv_direction, t_min, t_max)) {
            if(ptr == stack.begin()) break;
            current_node_index = *(--ptr);
            continue;
        }

        if(node.triangles_count == 0) {
            // 不是叶子节点
            if(dir_is_neg[node.split_axis]) {
                //光线的方向为负方向，先遍历右节点
                *(ptr++) = current_node_index + 1;
                current_node_index = node.child1_index;
            } else {
                //光线的方向为正方向，先遍历左节点
                current_node_index++;
                *(ptr++) = node.child1_index;
            }
            
        } else {
          // 是叶子节点
            auto triangle_iter = ordered_triangles.begin() + node.triangles_index;
            DEBUG_LINE(triangle_test_count += node.triangles_count)
            for(int i=0; i<node.triangles_count; i++)
            {
                auto hit_info = triangle_iter->intersect(ray, t_min, t_max);
                triangle_iter++;
                if(hit_info.has_value()) {
                    t_max = hit_info->t;
                    closest_hitinfo = hit_info;
                }
            }
            if(ptr == stack.begin()) break;
            current_node_index = *(--ptr);
        }
    }

    DEBUG_LINE(ray.bounds_test_count += bounds_test_count)
    DEBUG_LINE(ray.triangle_test_count += triangle_test_count)

    return closest_hitinfo;
}