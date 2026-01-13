//
// Created by simon on 13/01/2026.
//

#ifndef VULKAN_RTX_FOGCOMPONENT_HPP
#define VULKAN_RTX_FOGCOMPONENT_HPP
#include "Component.hpp"

#include <glm/vec4.hpp>

namespace ECS {
    struct FogComponent : public ECS::IComponent
    {
        float sigma_a{0.0001};  // not alterable through slider
        float sigma_s{0.0001};  // not alterable through slider
        float rayDistance{900.0f};
        float raySteps{4.0};

        glm::vec4 fogColour{0.0f};  // xyz - colour, w - density ,

        float heightFallOff{1};
        int   rayMarched      = false;
        float asymmetryFactor = {0.0f};
        float fogHeight       = {1.0f};

        FogComponent()
            : IComponent("Fog component", ICON_FA_SMOG)
        {
        }
    };
}  // namespace ECS

#endif  //VULKAN_RTX_FOGCOMPONENT_HPP
