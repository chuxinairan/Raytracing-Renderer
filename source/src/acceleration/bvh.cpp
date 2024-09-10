#include "acceleration/bvh.hpp"

void BVH::build(std::vector<Triangle> &&triangles)
{
    root = new BVHNode();
    root->triangles = std::move(triangles);
    root->updateBounds();
    recursiveSpilt(root);
}

void BVH::recursiveSpilt(BVHNode* node)
{
    if(node->triangles.size() == 1) {
        return;
    }

    auto diag = node->bounds.diagonal();
    int max_axis = diag.x > diag.y ? (diag.x > diag.z ? 0 : 2) : (diag.y > diag.z ? 1 : 2);
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
    BVHNode* child0 = new BVHNode();
    BVHNode* child1 = new BVHNode();
    node->children[0] = child0;
    node->children[1] = child1;
    node->triangles.clear();
    node->triangles.shrink_to_fit();
    child0->triangles = std::move(child0_triangles);
    child1->triangles = std::move(child1_triangles);
    child0->updateBounds();
    child1->updateBounds();
    
    recursiveSpilt(child0);
    recursiveSpilt(child1);
}

std::optional<HitInfo> BVH::intersect(Ray& ray, float t_min, float t_max) const 
{
    std::optional<HitInfo> closest_hitinfo;
    recursiveIntersect(root, ray, t_min, t_max, closest_hitinfo);
    return closest_hitinfo;
}

void BVH::recursiveIntersect(BVHNode* node, Ray& ray, float t_min, float &t_max, std::optional<HitInfo>& closest_hitinfo) const
{
    if(!node->bounds.hasIntersection(ray, t_min, t_max)) {
        return;
    }

    if(node->triangles.empty()) {
        // 不是叶子节点
        recursiveIntersect(node->children[0], ray, t_min, t_max, closest_hitinfo);
        recursiveIntersect(node->children[1], ray, t_min, t_max, closest_hitinfo);
    } else {
        // 是叶子节点
        for(const auto& triangle : node->triangles)
        {
            auto hit_info = triangle.intersect(ray, t_min, t_max);
            if(hit_info.has_value()) {
                t_max = hit_info->t;
                closest_hitinfo = hit_info;
            }
        }
    }
}