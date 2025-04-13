//
// Created by wpsimon09 on 12/04/25.
//

#include "VRayTracingBuilderKhr.hpp"

#include "VRayTracingBuilderKhrHelpers.hpp"
#include "VRayTracingStructs.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Scene/Scene.hpp"

namespace VulkanCore::RTX {
VRayTracingBuilderKHR::VRayTracingBuilderKHR(const VulkanCore::VDevice& device)
    : m_device(device)
{
  m_cmdPool   = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Graphics);
  m_cmdBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_cmdPool);
}

void VRayTracingBuilderKHR::BuildBLAS(std::vector<BLASInput>& inputs, vk::BuildAccelerationStructureFlagsKHR flags)
{
  m_blasEntries.reserve(inputs.size());
  for(auto& blas : inputs)
  {
    // make copy of the inputs
    BLASEntry entry;
    entry.input = blas;
    m_blasEntries.emplace_back(std::move(entry));
  }

  uint32_t       nbBlas = static_cast<uint32_t>(m_blasEntries.size());
  vk::DeviceSize asTotalSize{0};     // total size of the acceleration strcuture
  uint32_t       nbCompactions{0};   // number of BLAS that requested compaction
  vk::DeviceSize maxScratchSize{0};  // find the largest scratch size

  std::vector<VulkanCore::RTX::AccelerationStructBuildData> asBuildData(nbBlas);
  for(uint32_t i = 0; i < nbBlas; i++)
  {
    asBuildData[i].asType           = vk::AccelerationStructureTypeKHR::eBottomLevel;
    asBuildData[i].asGeometry       = m_blasEntries[i].input.asGeometry;
    asBuildData[i].asBuildRangeInfo = m_blasEntries[i].input.asBuildOffSetInfo;

    auto sizeInfo  = asBuildData[i].FinalizeGeometry(m_device, inputs[i].flags | flags);
    maxScratchSize = std::max(maxScratchSize, sizeInfo.buildScratchSize);
  }

  vk:VkDeviceSize hintMaxBudget{256'000'000};
  bool hasCompaction = hasFlag(static_cast<VkFlags>(flags), VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

  // scratch buffer needs to be used for every BLAS and we want ot reuse it so we will allocate scratch buffer with biggest size ever needed
  VulkanCore::VBuffer blasScratchBuffer(m_device, "BLAS Scratch buffer");
  blasScratchBuffer.CreateBuffer(maxScratchSize, static_cast<VkBufferUsageFlags>(vk::BufferUsageFlagBits::eShaderDeviceAddress| vk::BufferUsageFlagBits::eStorageBuffer));

  uint32_t minAlignment = GlobalVariables::GlobalStructs::AccelerationStructProperties.minAccelerationStructureScratchOffsetAlignment;
}

}  // namespace VulkanCore::RTX