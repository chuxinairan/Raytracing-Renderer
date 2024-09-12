#include "renderer/path_tracing_renderer.hpp"

#include "utils/frame.hpp"

#include "sample/spherical.hpp"

glm::vec3 PathTracingRenderer::renderPixel(const glm::ivec2 &pixelCoord) {
    auto& ray = camera.generateRay(pixelCoord, {rng.uniform(), rng.uniform()});
    glm::vec3 L = {0, 0, 0};
    glm::vec3 beta = {1, 1, 1};
    float q = 0.9;

    while (true) {
        auto hit_info = scene.intersect(ray);
        if (hit_info.has_value()) {
            if(q < rng.uniform()) {
                break;
            }

            glm::vec3 brdf;
            float pdf;
            Frame frame(hit_info->normal);
            glm::vec3 light_direction;
            if(hit_info->material->is_specular) {
                glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
                light_direction = glm::vec3(-view_direction.x, view_direction.y, -view_direction.z);
                pdf = 1.0f;
                brdf = hit_info->material->albedo / light_direction.y;
            } else {
                // 余弦重要性采样
                light_direction = CosineSampleHemisphere({rng.uniform(), rng.uniform()});
                pdf = light_direction.y / PI;
                brdf = hit_info->material->albedo / PI;

                // 接受拒绝采样采样
                // light_direction = UniformSampleHemisphere(rng);
                // pdf = 1 / (2 * PI);
                // brdf = hit_info->material->albedo / PI;
            }
            L += beta * hit_info->material->emissive;
            beta /= q;
            beta *= brdf * light_direction.y / pdf;

            ray.origin = hit_info->hit_point;
            ray.direction = frame.worldFromLocal(light_direction);
        } else {
            break;
        }
    }

    return L;
}