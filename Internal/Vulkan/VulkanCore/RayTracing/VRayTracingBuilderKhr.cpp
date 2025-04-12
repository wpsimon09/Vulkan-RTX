//
// Created by wpsimon09 on 12/04/25.
//

#include "VRayTracingBuilderKhr.hpp"

#include "VRayTracingBuilderKhrHelpers.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Scene/Scene.hpp"

namespace VulkanCore {
    VRayTracingBuilderKHR::VRayTracingBuilderKHR(const VulkanCore::VDevice& device, ApplicationCore::Scene& scene): m_device(device), m_scene(scene)
    {

    }

    void VRayTracingBuilderKHR::BuildBLAS()
    {
        std::vector<std::shared_ptr<ApplicationCore::StaticMesh>> meshes;
        m_scene.EnumarateMeshes(meshes, m_scene.GetRootNode());
        for (auto& mesh : meshes)
        {
            auto blas = StaticMeshToBLASInput(mesh);

            BLASEntry entry;
            entry.input = std::move(blas);
            m_blasEntries.emplace_back(std::move(entry));
        }
    }
} // VulkanCore