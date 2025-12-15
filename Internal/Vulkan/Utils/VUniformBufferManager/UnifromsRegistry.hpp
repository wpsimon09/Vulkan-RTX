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

struct GlobalRenderingInfo
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 inverseView;
    glm::mat4 inverseProj;

    glm::mat4 viewPrevFrame = glm::mat4(1.0);
    glm::mat4 projPrevFrame = glm::mat4(1.0);

    glm::vec4 atmosphereParams;
    glm::vec4 cameraPosition;
    glm::vec4 viewParams;   // x - image plane distance, y - focal length, z - near plane, w -far plane
    glm::vec4 viewParams2;  // x - FOV, y - aspect ratio, z - aparature size, w -cameraType

    float raysPerPixel;
    float reccursionDepth;
    alignas(8) glm::vec2 screenSize;

    float numberOfFrames;
    alignas(4) int rendererOutput;  // ClientEnums/EDebugRendering
    alignas(4) int rendererOutputRTX;

    alignas(4) int accumulateFrames                 = 1.0;
    alignas(4) int aoOcclusion                      = 1.0f;
    alignas(4) int isRayTracing                     = 0.0;
    alignas(4) int accountForAtmosphereTransmitance = 0.0;
    alignas(4) int useSer                           = 0.0;
    alignas(4) int useComposition                   = 1.0f;
    alignas(4) int useReflection                    = 1.0f;
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

struct RTXObjDescription
{
    vk::DeviceAddress vertexAddress;
    vk::DeviceAddress indexAddresss;
};

struct PerObjectPushConstant
{
    glm::ivec4 indexes{};  // x - object index, yzw - padding for now in future can be other indexes
    glm::mat4  modelMatrix{};
    glm::mat4  prevModelMatix{};
};


#endif  //UNIFROMDEFINITION_HPP
