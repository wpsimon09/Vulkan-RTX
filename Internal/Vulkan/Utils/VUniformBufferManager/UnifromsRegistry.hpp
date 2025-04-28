//
// Created by wpsimon09 on 31/10/24.
//

#ifndef UNIFROMDEFINITION_HPP
#define UNIFROMDEFINITION_HPP

#include <glm/glm.hpp>

#include "vulkan/vulkan.hpp"

namespace VulkanCore {
class VBuffer;
}

struct GlobalUniform
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 inverseView;
    glm::mat4 inverseProj;

    glm::vec4 lightPosition = glm::vec4(2.0f, 400.0f, 2.0f, 1.0f);
    glm::vec4 playerPosition;
    glm::vec4 viewParams;
    float     maxReccursion;
    float     reccursionDepth;
    glm::vec2 screenSize;
};

struct ObjectDataUniform
{
    glm::mat4              model;
    glm::mat4              normalMatrix;
    glm::vec3              position;
    float                  padding = 0.0f;
    PBRMaterialDescription material;
};

struct PointLightGPU
{
    glm::vec4 colour;           // w is intensity
    glm::vec4 position;         // w is in point light in use
    glm::vec4 CLQU_Parameters;  // x- constant, y - linear, z-quadratic, w - in use
};

struct DirectionalLightGPU
{
    glm::vec4 direction;
    glm::vec4 colour;  // w is intensity
};

struct AreaLightGPU
{
    glm::vec4                intensity;
    int                      isInUse;
    int                      twoSided;
    glm::vec2                padding;
    std::array<glm::vec4, 4> edges;
};

struct LightUniforms
{
    DirectionalLightGPU           directionalLight;
    std::array<PointLightGPU, 20> pointLight;
    std::array<AreaLightGPU, 4>   areaLights;
    glm::vec4                     info;  // x - use env, y - ambient strength,zw - padding
};

struct RTXObjDescription {
    vk::DeviceAddress vertexAddress;
    vk::DeviceAddress indexAddresss;
    // material
    // material index
    // etc etc
};


#endif  //UNIFROMDEFINITION_HPP
