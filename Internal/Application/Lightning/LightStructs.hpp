//
// Created by wpsimon09 on 03/02/25.
//

#ifndef LIGHTSTRUCTS_HPP
#define LIGHTSTRUCTS_HPP

#include <array>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <vector>

#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace VulkanCore {
class VImage2;
}

namespace LightStructs {

struct AreaLight
{

    glm::vec3 colour;
    float     intensity;
    glm::vec3 position;
    glm::vec2 scale;
    glm::quat orientation;

    bool                     twoSided;
    std::array<glm::vec4, 4> edges = {glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f},  // First point (bottom-left corner)
                                      glm::vec4{0.5f, -0.5f, 0.0f, 1.0f},   // Second point (bottom-right corner)
                                      glm::vec4{-0.5f, 0.5f, 0.0f, 1.0f},   // Third point (top-left corner)
                                      glm::vec4{0.5f, 0.5f, 0.0f, 1.0f}};

    bool isAreaLightInUse = false;

    void Reset()
    {
        colour           = glm::vec3(0.f);
        intensity        = 0.f;
        twoSided         = false;
        isAreaLightInUse = false;
        edges            = {glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f},  // First point (bottom-left corner)
                            glm::vec4{0.5f, -0.5f, 0.0f, 1.0f},   // Second point (bottom-right corner)
                            glm::vec4{-0.5f, 0.5f, 0.0f, 1.0f},   // Third point (top-left corner)
                            glm::vec4{0.5f, 0.5f, 0.0f, 1.0f}};
    }

    std::array<glm::vec4, 4> GetAreaLightEdges()
    {
        return {glm::vec4{-0.5, -0.5f, 0.0f, 1.0f},  // First point (bottom-left corner)
                glm::vec4{0.5, -0.5f, 0.0f, 1.0f},   // Second point (bottom-right corner)
                glm::vec4{0.5, 0.5f, 0.0f, 1.0f},    // Third point (top-left corner)
                glm::vec4{-0.5, 0.5f, 0.0f, 1.0f}};
    }

    friend bool operator==(const AreaLight& lhs, const AreaLightGPU& rhs)
    {
        return lhs.intensity == rhs.intensity.w && lhs.colour.x == rhs.intensity.x && lhs.colour.y == rhs.intensity.y
               && lhs.colour.z == rhs.intensity.z && lhs.isAreaLightInUse == rhs.isInUse
               && lhs.twoSided == static_cast<bool>(rhs.twoSided) && lhs.edges == rhs.edges;
    }

    friend bool operator!=(const AreaLight& lhs, const AreaLightGPU& rhs) { return !(lhs == rhs); }
};

struct PointLight
{


    mutable glm::vec4 colour;

    mutable glm::vec3 position;

    float constantFactor = 1.0f;

    float linearFactor = 0.045f;

    float quadraticFactor = 0.0075f;

    bool useAdvancedAttentuation = 0;
    bool isPointLightInUse       = false;

    void Reset()
    {
        colour            = glm::vec4(0.f);
        position          = glm::vec3(0.f);
        constantFactor    = 1.0f;
        linearFactor      = 0.045f;
        quadraticFactor   = 0.0075f;
        isPointLightInUse = false;
    }

    friend bool operator==(const PointLight& lhs, const PointLightGPU& rhs)
    {
        return lhs.colour == rhs.colour && lhs.position.x == rhs.position.x && lhs.position.y == rhs.position.y
               && lhs.position.z == rhs.position.z && lhs.constantFactor == rhs.CLQU_Parameters.x
               && lhs.linearFactor == rhs.CLQU_Parameters.y && lhs.quadraticFactor == rhs.CLQU_Parameters.z
               && lhs.isPointLightInUse == rhs.CLQU_Parameters.w;
    }

    friend bool operator!=(const PointLight& lhs, const PointLightGPU& rhs) { return !(lhs == rhs); }
};

struct DirectionalLight
{
    glm::vec4 colour;

    mutable glm::vec3 direction;

    float sunRadius = 0.1f;

    int shadowRaysPerPixel{2};

    float shadowBias{0.1};

    void Reset()
    {
        colour    = glm::vec4(0.f);
        direction = glm::vec3(0.f);
    }
    bool inUse = true;

    friend bool operator==(const DirectionalLight& lhs, const DirectionalLightGPU& rhs)
    {
        return lhs.colour == rhs.colour && glm::vec4(lhs.direction, 1.0f) == rhs.direction;
    }

    friend bool operator!=(const DirectionalLight& lhs, const DirectionalLightGPU& rhs) { return !(lhs == rhs); }
};

struct EnvLight
{
    std::shared_ptr<ApplicationCore::VTextureAsset> hdrImage;
    float                                           ambientIntensity = 0.07;
    bool                                            inUse            = true;
    std::string                                     hdrPath;
};

struct SceneLightInfo
{

    LightStructs::DirectionalLight*        DirectionalLightInfo = nullptr;
    std::vector<LightStructs::PointLight*> PointLightInfos;
    std::vector<LightStructs::AreaLight*>  AreaLightInfos;
    EnvLight*                              environmentLight = nullptr;
    float                                  ambientStrenght  = 0.4;

    int AddPointLight(PointLight* pointLight)
    {
        PointLightInfos.emplace_back(pointLight);
        pointLight->isPointLightInUse = true;
        CurrentPointLightIndex++;
        return CurrentPointLightIndex - 1;
    };

    int AddAreaLight(AreaLight* areaLight)
    {
        AreaLightInfos.emplace_back(areaLight);
        areaLight->isAreaLightInUse = true;
        CurrentAreaLightIndex++;
        return CurrentPointLightIndex - 1;
    };

  private:
    int CurrentPointLightIndex = 0;
    int CurrentAreaLightIndex  = 0;
};


}  // namespace LightStructs

#endif  // LIGHTSTRUCTS_HPP
