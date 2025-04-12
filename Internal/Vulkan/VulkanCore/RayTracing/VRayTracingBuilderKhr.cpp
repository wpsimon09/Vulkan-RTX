//
// Created by wpsimon09 on 12/04/25.
//

#include "VRayTracingBuilderKhr.hpp"

#include "VRayTracingBuilderKhrHelpers.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Scene/Scene.hpp"

namespace VulkanCore {
    VRayTracingBuilderKHR::VRayTracingBuilderKHR(const VulkanCore::VDevice& device): m_device(device)
    {

    }

    void VRayTracingBuilderKHR::BuildBLAS(std::vector<BLASInput>& inputs, vk::BuildAccelerationStructureFlagsKHR flags)
    {
        m_blasEntries.reserve(inputs.size());
        for (auto& blas : inputs)
        {
            // make copy of the inputs
            BLASEntry entry;
            entry.input = blas;
            m_blasEntries.emplace_back(std::move(entry));
        }
    }

} // VulkanCore