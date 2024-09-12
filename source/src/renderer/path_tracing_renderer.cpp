#include "renderer/path_tracing_renderer.hpp"

#include "utils/frame.hpp"

#include "sample/spherical.hpp"

glm::vec3 PathTracingRenderer::renderPixel(const glm::ivec2 &pixelCoord) {
    auto& ray = camera.generateRay(pixelCoord, {rng.uniform(), rng.uniform()});
    glm::vec3 L = {0, 0, 0};
    glm::vec3 beta = {1, 1, 1};
    float q = 0.9f;

    while (true) {
        auto hit_info = scene.intersect(ray);
        if (hit_info.has_value()) {
            if(q < rng.uniform()) {
                break;
            }
            beta /= q;
            L += beta * hit_info->material->emissive;
        
            Frame frame(hit_info->normal);
            glm::vec3 light_direction;
            if(hit_info->material) {
                glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
                light_direction = hit_info->material->sampleBRDF(view_direction, beta, rng);
            } else {
                break;
            } 

            ray.origin = hit_info->hit_point;
            ray.direction = frame.worldFromLocal(light_direction);
        } else {
            break;
        }
    }

    return L;
}