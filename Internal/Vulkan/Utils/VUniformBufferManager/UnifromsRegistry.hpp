//
// Created by wpsimon09 on 31/10/24.
//

#ifndef UNIFROMDEFINITION_HPP
#define UNIFROMDEFINITION_HPP

#include <glm/glm.hpp>

#include "Application/Rendering/Material/MaterialStructs.hpp"
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
    glm::vec4 cameraPosition;
    glm::vec4 viewParams;

    float     raysPerPixel;
    float     reccursionDepth;
    glm::vec2 screenSize;
    float     numberOfFrames;
    float     padding1, padding2, padding3;
};

struct ObjectDataUniform
{
    glm::mat4              model;
    glm::mat4              normalMatrix;
    glm::vec3              position;
    float                  padding = 0.0f;
    PBRMaterialDescription material;
};

struct PerObjectData
{
    glm::mat4  model{};
    glm::mat4  normalMatrix{};
    glm::vec4  position{};
    glm::uvec4 indexes{};  // x - material index,
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
    glm::vec4 colour;      // w is intensity
    glm::vec4 parameters;  // x - shadow rays, y - shadow bias ,zw - unused
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
    glm::vec4                     info;  // x - use env, y - ambient strength,w - padding
};

struct FogVolumeParameters
{
    float sigma_a{0.1};
    float sigma_s{0.1};
    float rayDistance{900.0f};
    float raySteps{4.0};

    glm::vec4 fogColour{0.0f};

    float heightFallOff{1};
    int   rayMarched      = false;
    float asymmetryFactor = {0.0f};
    float fogHeight       = {1.0f};
};

struct RTXObjDescription
{
    vk::DeviceAddress vertexAddress;
    vk::DeviceAddress indexAddresss;
};

struct PerObjectPushConstant
{
    glm::ivec4 indexes{};  // x - object index, yzw - padding for now in future can be other indexes
    glm::mat4  modelMatrix{};
};

struct PostProcessingParameters
{
    glm::vec4 toneMapping{1.0, 2.2, 0.0, 0.0};  // x- exposure, y - gamma_correction, zw - padding
    float     lensFlareStrength = 2.0f;
    float     f1Strength        = 2.0f;
    float     f2Strength        = 2.0f;
    float     f3Strength        = 1.0f;
};


#endif  //UNIFROMDEFINITION_HPP
