#include "acceleration/scene_bvh.hpp"

#include <array>
#include <iostream>

#include "utils/debug_macro.hpp"
#include "utils/profile.hpp"

void SceneBVH::build(std::vector<ShapeInstance> &&instances)
{
    auto* root = allocator.allocate();

    auto temp_instances = std::move(instances);
    for (auto &instance : temp_instances) {
        if (instance.shape.getBounds().isValid()) {
            instance.updateBounds();
            root->instances.push_back(instance);
        } else {
            infinity_instances.push_back(instance);
        }
    }
    
    root->updateBounds();
    root->depth = 1;
    SceneBVHState state {};
    size_t instance_count = root->instances.size();
    recursiveSpilt(root, state);

    std::cout << "Total_Node_Count: " << state.total_node_count << std::endl;
    std::cout << "Leaf_Node_Count: " << state.leaf_node_count << std::endl;
    std::cout << "Total_Shape_instance_Count: " << instance_count << std::endl;
    std::cout << "Mean_leaf_node_instance_count: " << static_cast<float>(instance_count) / state.leaf_node_count << std::endl;
    std::cout << "Max_leaf_node_instance_count: " << state.max_leaf_node_instance_count << std::endl;
    std::cout << "Max_leaf_node_depth: " << state.max_leaf_node_depth << std::endl;

    nodes.reserve(state.total_node_count);
    ordered_instances.reserve(instance_count);
    recursiveFlatten(root);
}

void SceneBVH::recursiveSpilt(SceneBVHTreeNode* node, SceneBVHState& state)
{
    state.total_node_count++;
    if(node->instances.size() == 1 || node->depth > 32) {
        state.addLeafNode(node);
        return;
    }

    auto diag = node->bounds.diagonal();
    constexpr size_t bucket_count = 12;
    size_t min_split_index = 0;
    float min_cost = std::numeric_limits<float>::infinity();
    Bounds min_child0_bounds{}, min_child1_bounds{};
    size_t min_child0_instance_count = 0, min_child1_instance_count = 0;
    std::vector<size_t> instance_indices_per_bucket[3][bucket_count] = {};
    for(size_t axis = 0; axis < 3; axis++) {
        size_t instance_count_per_bucket[bucket_count] = {};
        Bounds bucket_bounds[bucket_count] = {};
        size_t instance_idx = 0;
        for(const auto& instance : node->instances) {
            size_t bucket_idx = glm::clamp<size_t>(
                glm::floor((instance.center[axis] - node->bounds.b_min[axis]) * bucket_count / diag[axis]),
                0, bucket_count - 1
            );
            bucket_bounds[bucket_idx].expand(instance.bounds);
            instance_count_per_bucket[bucket_idx]++;
            instance_indices_per_bucket[axis][bucket_idx].push_back(instance_idx);
            instance_idx++;
        }

        Bounds left_bounds = bucket_bounds[0];
        size_t left_instance_count = instance_count_per_bucket[0];
        for(size_t i = 1; i < bucket_count; i++) {
            Bounds right_bounds{};
            size_t right_instance_count = 0;
            for(size_t j = bucket_count-1; j >= i; j--) {
                right_bounds.expand(bucket_bounds[j]);
                right_instance_count += instance_count_per_bucket[j];
            }

            if(right_instance_count == 0) {
                break;
            }
            if(left_instance_count != 0) {
                float cost = left_instance_count * left_bounds.area() + right_instance_count * right_bounds.area();
                if(cost < min_cost) {
                    min_cost = cost;
                    min_split_index = i;
                    node->split_axis = axis;
                    min_child0_bounds = left_bounds;
                    min_child1_bounds = right_bounds;
                    min_child0_instance_count = left_instance_count;
                    min_child1_instance_count = right_instance_count;
                }
            }
            left_bounds.expand(bucket_bounds[i]);
            left_instance_count += instance_count_per_bucket[i];
        }
    }

    if(min_split_index == 0) {
        state.addLeafNode(node);
        return;
    }

    SceneBVHTreeNode* child0 = allocator.allocate();
    SceneBVHTreeNode* child1 = allocator.allocate();

    node->children[0] = child0;
    node->children[1] = child1;
    node->children[0]->depth = node->depth + 1;
    node->children[1]->depth = node->depth + 1;

    child0->instances.reserve(min_child0_instance_count);
    for(size_t i = 0; i < min_split_index; i++) {
        for(size_t idx : instance_indices_per_bucket[node->split_axis][i]) {
            child0->instances.push_back(node->instances[idx]);
        }
    }
    child1->instances.reserve(min_child1_instance_count);
    for(size_t i = min_split_index; i < bucket_count; i++) {
        for(size_t idx : instance_indices_per_bucket[node->split_axis][i]) {
            child1->instances.push_back(node->instances[idx]);
        }
    }

    node->instances.clear();
    node->instances.shrink_to_fit();

    child0->bounds = min_child0_bounds;
    child1->bounds = min_child1_bounds;

    recursiveSpilt(child0, state);
    recursiveSpilt(child1, state);
}


size_t SceneBVH::recursiveFlatten(SceneBVHTreeNode* node)
{
    SceneBVHNode linear_node{
        node->bounds,
        0,
        static_cast<uint16_t>(node->instances.size()),
        static_cast<uint8_t>(node->split_axis)
    };
    size_t idx = nodes.size();
    nodes.push_back(linear_node);
    if(nodes[idx].instances_count == 0) {
        recursiveFlatten(node->children[0]);
        nodes[idx].child1_index = recursiveFlatten(node->children[1]);
    } else {
        nodes[idx].instances_index = ordered_instances.size();
        for(const auto& instance : node->instances) {
            ordered_instances.push_back(std::move(instance));
        }
    }
    return idx;
}

std::optional<HitInfo> SceneBVH::intersect(const Ray& ray, float t_min, float t_max) const 
{
    DEBUG_LINE(size_t bounds_test_count = 0)
    std::optional<HitInfo> closest_hit_info;
    const ShapeInstance *closest_instance = nullptr;

    std::array<int, 32> stack;
    auto ptr = stack.begin();

    glm::bvec3 dir_is_neg {
        ray.direction.x < 0,
        ray.direction.y < 0,
        ray.direction.z < 0
    };
    glm::vec3 inv_direction = 1.0f / ray.direction;

    size_t current_node_index = 0;
    while(true) {
        auto& node = nodes[current_node_index];
        DEBUG_LINE(bounds_test_count++)
        if(!node.bounds.hasIntersection(ray, inv_direction, t_min, t_max)) {
            if(ptr == stack.begin()) break;
            current_node_index = *(--ptr);
            continue;
        }

        if(node.instances_count == 0) {
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
            auto instance_iter = ordered_instances.begin() + node.instances_index;
            for(size_t i= 0 ; i < node.instances_count; i++)
            {
                Ray obj_ray = ray.objectFromWorld(instance_iter->object_from_world);
                auto hit_info = instance_iter->shape.intersect(obj_ray, t_min, t_max);
                DEBUG_LINE(ray.bounds_test_count += obj_ray.bounds_test_count)
                DEBUG_LINE(ray.triangle_test_count += obj_ray.triangle_test_count)
                if(hit_info.has_value()) {
                    t_max = hit_info->t;
                    closest_hit_info = hit_info;
                    closest_instance = &(*instance_iter);
                }
                ++instance_iter;
            }
            if(ptr == stack.begin()) break;
            current_node_index = *(--ptr);
        }
    }

    for(const auto &infinity_instance : infinity_instances) {
        Ray obj_ray = ray.objectFromWorld(infinity_instance.object_from_world);
        auto hit_info = infinity_instance.shape.intersect(obj_ray, t_min, t_max);
        DEBUG_LINE(ray.bounds_test_count += obj_ray.bounds_test_count)
        DEBUG_LINE(ray.triangle_test_count += obj_ray.triangle_test_count)
        if(hit_info.has_value()) {
            t_max = hit_info->t;
            closest_hit_info = hit_info;
            closest_instance = &infinity_instance;
        }
    }

    if (closest_instance) {
        closest_hit_info->hit_point = closest_instance->world_from_object * glm::vec4(closest_hit_info->hit_point, 1.0f);

        closest_hit_info->normal =glm::normalize(glm::vec3(glm::transpose(closest_instance->object_from_world) * glm::vec4(closest_hit_info->normal, 0.f)));
        closest_hit_info->material = &closest_instance->material;
    }
    DEBUG_LINE(ray.bounds_test_count += bounds_test_count)

    return closest_hit_info;
}