//
// Created by wpsimon09 on 31/10/24.
//

#ifndef UNIFROMDEFINITION_HPP
#define UNIFROMDEFINITION_HPP
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "vulkan/vulkan.hpp"

namespace VulkanCore
{
    class VBuffer;
}

struct GlobalUniform
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 inverseView;

    glm::vec4 lightPosition = glm::vec4(2.0f, 400.0f, 2.0f,1.0f);
    glm::vec4 playerPosition;
    glm::vec4 viewParams;
    float padding;
    float padding2;
    glm::vec2 screenSize;
};

struct ObjectDataUniform
{
    glm::mat4 model;
    glm::mat4 normalMatrix;
    glm::vec3 position;
};

struct PointLightGPU
{
    int count;
    float padding[3];
    glm::vec4 colour {0.2f, 0.2f, 0.4f, 10.0f}; // w is intensity
    glm::vec4 position;
};

struct DirectionalLightGPU
{
    glm::vec4 direction;
    glm::vec4 colour; // w is intensity
};

struct LightUniforms
{
    DirectionalLightGPU directionalLight;
    std::array<PointLightGPU, 10> pointLight;
};


#endif //UNIFROMDEFINITION_HPP
