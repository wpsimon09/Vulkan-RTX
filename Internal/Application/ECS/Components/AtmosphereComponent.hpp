//
// Created by simon on 13/01/2026.
//

#ifndef VULKAN_RTX_ATMOSPHERECOMPONENT_HPP
#define VULKAN_RTX_ATMOSPHERECOMPONENT_HPP
#include "Component.hpp"

#include <glm/vec4.hpp>

namespace ECS {
struct AtmosphereComponent : public ECS::IComponent
{
    glm::vec4 groundAlbedo = {0.3, 0.3, 0.3, 6360};                         // xyz: as-is, w: Radius bottom
    glm::vec4 rayleighScattering{0.005802f, 0.013558f, 0.033100f, 6460.0};  // xyz: wevelength dependant, w: Radius top
    // rayleightAbsorption is 0 in paper so not included here
    glm::vec4 mieScattering = {0.003996f, 0.003996f, 0.003996f, 8.0};  // xyz: as-is, w: Rayleigh density exp scale
    glm::vec4 mieAbsorption = {0.004440f, 0.004440f, 0.004440f, 1.2};  // xyz: as-is, w: Mie density exp scale
    glm::vec4 mieExtinction = {0.004440f, 0.004440f, 0.004440f, 0.8};  // xyz: as-is, w: Mie phase g
    glm::vec4 absorptionExtinction = {0.000650f, 0.001881f, 0.000085f, 0.0f};  // xyz: as-is, w: Absorption density 0 layer width
    glm::ivec4 booleans = {0.0, 0.0, 0.0, 0.0};

    AtmosphereComponent()
        : IComponent("AtmosphereComponent", ICON_FA_MOUNTAIN_SUN)
    {
    }
};
}  // namespace ECS

#endif  //VULKAN_RTX_ATMOSPHERECOMPONENT_HPP
