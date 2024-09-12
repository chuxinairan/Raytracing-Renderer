#include "renderer/debug_renderer.hpp"

#include "utils/rgb.hpp"

glm::vec3 BoundsTestCountRenderer::renderPixel(const glm::ivec2 &pixelCoord) {
    #ifdef WITH_DEBUG_INFO
        auto ray = camera.generateRay(pixelCoord);
        auto hitinfo = scene.intersect(ray);
        return RGB::GenerateHeatmapRGB(ray.bounds_test_count / 150.0f);
        return {};
    #else
        return {};
    #endif
}

glm::vec3 TriangleTestCountRenderer::renderPixel(const glm::ivec2 &pixelCoord) {
    #ifdef WITH_DEBUG_INFO
        auto ray = camera.generateRay(pixelCoord);
        auto hitinfo = scene.intersect(ray);
        return RGB::GenerateHeatmapRGB(ray.triangle_test_count / 7.0f);
        return {};
    #else
        return {};
    #endif    
}