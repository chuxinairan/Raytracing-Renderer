#include "acceleration/bvh.hpp"

#include <array>

#include "utils/debug_macro.hpp"

void BVH::build(std::vector<Triangle> &&triangles)
{
    auto* root = new BVHTreeNode();
    root->triangles = std::move(triangles);
    root->updateBounds();
    root->depth = 1;
    recursiveSpilt(root);

    recursiveFlatten(root);
}

void BVH::recursiveSpilt(BVHTreeNode* node)
{
    if(node->triangles.size() == 1 || node->depth > 32) {
        return;
    }

    auto diag = node->bounds.diagonal();
    int max_axis = diag.x > diag.y ? (diag.x > diag.z ? 0 : 2) : (diag.y > diag.z ? 1 : 2);
    node->split_axis = max_axis;
    float mid = node->bounds.b_min[max_axis] + diag[max_axis] * 0.5f;
    std::vector<Triangle> child0_triangles, child1_triangles;
    for(const auto& triangle : node->triangles)
    {
        if((triangle.p0[max_axis] + triangle.p1[max_axis] + triangle.p2[max_axis]) / 3.0f < mid) {
            child0_triangles.push_back(triangle); 
        } else {
            child1_triangles.push_back(triangle);
        }
    }
    if(child0_triangles.empty() || child1_triangles.empty()) {
        return;
    }
    BVHTreeNode* child0 = new BVHTreeNode();
    BVHTreeNode* child1 = new BVHTreeNode();
    node->children[0] = child0;
    node->children[1] = child1;
    node->children[0]->depth = node->depth + 1;
    node->children[1]->depth = node->depth + 1;
    node->triangles.clear();
    node->triangles.shrink_to_fit();
    child0->triangles = std::move(child0_triangles);
    child1->triangles = std::move(child1_triangles);
    child0->updateBounds();
    child1->updateBounds();
    
    recursiveSpilt(child0);
    recursiveSpilt(child1);
}


size_t BVH::recursiveFlatten(BVHTreeNode* node)
{
    BVHNode linear_node{
        node->bounds,
        0,
        static_cast<uint16_t>(node->triangles.size()),
        static_cast<uint8_t>(node->depth),
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

std::optional<HitInfo> BVH::intersect(Ray& ray, float t_min, float t_max) const 
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
                    DEBUG_LINE(closest_hitinfo->bounds_depth = node.depth)
                }
            }
            if(ptr == stack.begin()) break;
            current_node_index = *(--ptr);
        }
    }
    if(closest_hitinfo.has_value()) {
        DEBUG_LINE(closest_hitinfo->bounds_test_count = bounds_test_count)
        DEBUG_LINE(closest_hitinfo->triangle_test_count = triangle_test_count)
    }

    return closest_hitinfo;
}