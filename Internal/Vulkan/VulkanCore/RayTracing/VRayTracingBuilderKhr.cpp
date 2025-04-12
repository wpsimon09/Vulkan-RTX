//
// Created by wpsimon09 on 12/04/25.
//

#include "VRayTracingBuilderKhr.hpp"

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Scene/Scene.hpp"

namespace VulkanCore {
    VRayTracingBuilderKHR::VRayTracingBuilderKHR(const VulkanCore::VDevice& device, ApplicationCore::Scene& scene): m_device(device), m_scene(scene)
    {
        for (auto& mesh : m_scene.GetAssetsManager().GetMeshes())
        {
            // store the
        }
    }
} // VulkanCore