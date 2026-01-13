//
// Created by simon on 13/01/2026.
//

#ifndef VULKAN_RTX_LIGHTCOMPONENTS_HPP
#define VULKAN_RTX_LIGHTCOMPONENTS_HPP
#include "Component.hpp"
#include "vulkan/vulkan.hpp"
#include <array>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

namespace ApplicationCore {
class VTextureAsset;
}
struct DirectionalLightComponent : public ECS::IComponent
{
    glm::vec4 colour             = {1.0};
    glm::vec4 direction          = {0.0, -1.0, 0.0, 0.0};
    float     radius             = {0.01};
    int       shadowRaysPerPixel = {2};
    float     shadowBias         = {0.1};
    bool      inUse              = true;
    DirectionalLightComponent()
        : IComponent("Directional light component", ICON_FA_SUN)
    {
    }
};

struct PointLightComponent : public ECS::IComponent
{

    glm::vec4 colour;
    glm::vec4 position;

    float constantFactor  = 1.0f;
    float linearFactor    = 0.045f;
    float quadraticFactor = 0.0075f;

    bool useAdvancedAttentuation = 0;
    bool isPointLightInUse       = false;

    /*
     *Returns constant factor, linear factor, quadratic factor into the single glm::vec4
     */
    glm::vec4 PackTerms() const
    {
        return glm::vec4(constantFactor, linearFactor, quadraticFactor, useAdvancedAttentuation);
    }
    PointLightComponent()
        : IComponent("Point light component", ICON_FA_LIGHTBULB)
    {
    }
};

struct AreaLightComponent : public ECS::IComponent
{
    glm::vec4 colour   = {1.0};
    bool      twoSided = false;

    std::array<glm::vec4, 4> edges = {glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f},  // First point (bottom-left corner)
                                      glm::vec4{0.5f, -0.5f, 0.0f, 1.0f},   // Second point (bottom-right corner)
                                      glm::vec4{-0.5f, 0.5f, 0.0f, 1.0f},   // Third point (top-left corner)
                                      glm::vec4{0.5f, 0.5f, 0.0f, 1.0f}};
    void                     ApplyTransform(glm::mat4& transform)
    {
        for(auto& edge : edges)
        {
            edge = transform * edge;
        }
    }

    std::array<glm::vec4, 4>& GetEdges() { return edges; }
};

struct EnvironemntLightComponent
{
    std::shared_ptr<ApplicationCore::VTextureAsset> hdrImage;
    // for preview in ImGui
    // VEditor::ImGuiImagae
    float ambientIntensity;
};

#endif  //VULKAN_RTX_LIGHTCOMPONENTS_HPP
