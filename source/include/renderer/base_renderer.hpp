#pragma once

#include <glm/glm.hpp>

#include "camera/camera.hpp"
#include "shape/scene.hpp"
#include "utils/rng.hpp"

#define DEFINE_RENDERER(Name) \
class Name##Renderer : public BaseRenderer \
{ \
    public: \
        Name##Renderer(Camera& camera, const Scene& scene) : BaseRenderer(camera, scene) {} \
    private: \
        glm::vec3 renderPixel(const glm::ivec2& pixelCoord) override; \
}; \

class BaseRenderer
{
public:
    BaseRenderer(Camera& camera, const Scene& scene) : camera(camera), scene(scene) {}

    void render(size_t spp, const std::filesystem::path& filename);
protected:
    Camera& camera;
    const Scene& scene;
    RNG rng{};
private:
    virtual glm::vec3 renderPixel(const glm::ivec2& pixelCoord) = 0;
};