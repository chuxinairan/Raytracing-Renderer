#include "renderer/debug_renderer.hpp"

#include "utils/rgb.hpp"

glm::vec3 BoundsTestCountRenderer::renderPixel(const glm::ivec2 &pixelCoord) {
    #ifdef WITH_DEBUG_INFO
        auto ray = camera.generateRay(pixelCoord);
        auto hitinfo = scene.intersect(ray);
        if(hitinfo.has_value()) {
            return RGB::GenerateHeatmapRGB(hitinfo->bounds_test_count / 150.0f);
        }
        return {};
    #else
        return {};
    #endif
}

glm::vec3 TriangleTestCountRenderer::renderPixel(const glm::ivec2 &pixelCoord) {
    #ifdef WITH_DEBUG_INFO
        auto ray = camera.generateRay(pixelCoord);
        auto hitinfo = scene.intersect(ray);
        if(hitinfo.has_value()) {
            return RGB::GenerateHeatmapRGB(hitinfo->triangle_test_count / 7.0f);
        }
        return {};
    #else
        return {}
    #endif    
}

glm::vec3 BoundsDepthRenderer::renderPixel(const glm::ivec2 &pixelCoord) {
    #ifdef WITH_DEBUG_INFO
        auto ray = camera.generateRay(pixelCoord);
        auto hitinfo = scene.intersect(ray);
        if(hitinfo.has_value()) {
            return RGB::GenerateHeatmapRGB(hitinfo->bounds_depth / 32.0f);
        }
        return {};
    #else
        return {}
    #endif    
}