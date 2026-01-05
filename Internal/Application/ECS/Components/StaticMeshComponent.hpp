//
// Created by simon on 03/01/2026.
//

#ifndef VULKAN_RTX_STATICMESHCOMPONENT_HPP
#define VULKAN_RTX_STATICMESHCOMPONENT_HPP
#include "Component.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"

namespace ECS {

struct StaticMeshComponent : public ECS::IComponent
{
    char                              meshName[100] = "unknown";
    VulkanStructs::VGPUSubBufferInfo* vertexData    = nullptr;
    VulkanStructs::VGPUSubBufferInfo* indexData     = nullptr;
    VulkanStructs::VBounds            bounds        = {};
    uint32_t                          indexCount    = 0;
    vk::DeviceAddress                 deviceAddress = 0;

    StaticMeshComponent()
        : ECS::IComponent("Static mesh component", ICON_FA_CUBE)
    {
    }
    bool IsValid() { return vertexData != nullptr && indexData != nullptr; }

    friend bool operator==(const StaticMeshComponent& lhs, const StaticMeshComponent& rhs)
    {
        return lhs.vertexData == rhs.vertexData && lhs.indexData == rhs.indexData && lhs.indexCount == rhs.indexCount
               && lhs.deviceAddress == rhs.deviceAddress;
    }
    friend bool operator!=(const StaticMeshComponent& lhs, const StaticMeshComponent& rhs) { return !(lhs == rhs); }
};
}  // namespace ECS

#endif  //VULKAN_RTX_STATICMESHCOMPONENT_HPP
