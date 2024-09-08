#pragma once

#include <glm/glm.hpp>

class RGB
{
public:
    RGB(int r, int g, int b) : r(r), g(g), b(b) {}

    // 真实物理世界的光照强度做gamma矫正
    RGB(const glm::vec3 &color) {
        r = glm::clamp<int>(glm::pow(color.x, 1.0f / 2.2f) * 255, 0, 255);
        g = glm::clamp<int>(glm::pow(color.y, 1.0f / 2.2f) * 255, 0, 255);
        b = glm::clamp<int>(glm::pow(color.z, 1.0f / 2.2f) * 255, 0, 255);
    }

    // sRGB是处在gamma0.45空间下的，这里做的是做gamma correction remove，来变换到物理线性空间
    operator glm::vec3() {
        return glm::vec3{
            glm::pow(r / 255.0f, 2.2f),
            glm::pow(g / 255.0f, 2.2f),
            glm::pow(b / 255.0f, 2.2f)
        };
    }
public:
    int r, g, b;
};