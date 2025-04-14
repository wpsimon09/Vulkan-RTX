//
// Created by wpsimon09 on 13/04/25.
//

#include "VRayTracingBlasBuilder.hpp"

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
namespace RTX {
VRayTracingBlasBuilder::VRayTracingBlasBuilder(const VulkanCore::VDevice& device):m_device(device) {}

bool VRayTracingBlasBuilder::CmdCreateBlas(const VulkanCore::VCommandBuffer&                          cmdBuffer,
                                           std::vector<VulkanCore::RTX::AccelerationStructBuildData>& buildInfo,
                                           std::vector<VulkanCore::RTX::AccelKHR>&                    outAs,
                                           vk::DeviceAddress                                          scratchAdress,
                                           vk::DeviceSize                                             hintMaxBudget)
{
  std::vector<vk::DeviceAddress> scratchAdresses = {scratchAdress};
  return CmdCreateParallelBlas(cmdBuffer, buildInfo, outAs, scratchAdresses, hintMaxBudget);
}

bool VRayTracingBlasBuilder::CmdCreateParallelBlas(const VulkanCore::VCommandBuffer&                          cmdBuffer,
                                                   std::vector<VulkanCore::RTX::AccelerationStructBuildData>& buildInfo,
                                                   std::vector<VulkanCore::RTX::AccelKHR>&                    outAs,
                                                   std::vector<vk::DeviceAddress>& scratchAdresses,
                                                   vk::DeviceSize    hintMaxBudget)
{

  InitializeQueryPoolIfNeeded(buildInfo);
  vk::DeviceSize  processBudget = 0; // all memroy consumed during construction
  uint32_t        currentQueryIndex = m_currentQueryIndex;

  while (m_currentBlasIndex < buildInfo.size() && processBudget < hintMaxBudget) {
    processBudget += BuildAccelerationStructures(cmdBuffer, buildInfo, outAs, scratchAdresses, hintMaxBudget, processBudget, currentQueryIndex);
  }

  return m_currentBlasIndex >= buildInfo.size();

}

void VRayTracingBlasBuilder::CmdCompactBlas(const VulkanCore::VCommandBuffer&         cmdBuffer,
                                            std::vector<AccelerationStructBuildData>& blasBuildData,
                                            std::vector<AccelKHR>&                    outBlas)
{

}

void           VRayTracingBlasBuilder::DestroyNonCompactedBlas() {}

vk::DeviceSize VRayTracingBlasBuilder::GetScratchSize(vk::DeviceSize                                  hintMaxBudget,
                                                      const std::vector<AccelerationStructBuildData>& blasBuildData,
                                                      uint32_t                                        minAlignment)
{

}

void VRayTracingBlasBuilder::GetScratchAddresses(vk::DeviceSize                                  hintMaxBudget,
                                                 const std::vector<AccelerationStructBuildData>& blasBuildData,
                                                 vk::DeviceAddress                               scratchBufferAderess,
                                                 std::vector<vk::DeviceAddress>&                 outScratchAddresses,
                                                 uint32_t                                        minimumAligment)
{
}

void VRayTracingBlasBuilder::Destroy() {}

void VRayTracingBlasBuilder::DestroyQueryPool() {}

void VRayTracingBlasBuilder::CreateQueryPool(uint32_t maxBlasCount) {
  vk::QueryPoolCreateInfo queryPoolCI {};
  queryPoolCI.queryType = vk::QueryType::eAccelerationStructureCompactedSizeKHR;
  queryPoolCI.queryCount = maxBlasCount;
  assert(m_device.GetDevice().createQueryPool(&queryPoolCI, nullptr, &m_queryPool) == vk::Result::eSuccess && "Failed to creat query pool !");

}

void VRayTracingBlasBuilder::InitializeQueryPoolIfNeeded(const std::vector<AccelerationStructBuildData>& blasBuildData)
{
}

vk::DeviceSize VRayTracingBlasBuilder::BuildAccelerationStructures(const VulkanCore::VCommandBuffer& cmdBuffer,
                                                                   std::vector<AccelerationStructBuildData>& blasBuildData,
                                                                   std::vector<AccelKHR>&                outAccel,
                                                                   const std::vector<vk::DeviceAddress>& scratchAdress,
                                                                   vk::DeviceSize                        hintMaxBudget,
                                                                   vk::DeviceSize                        currentBudget,
                                                                   uint32_t& currentQueryIndex)
{

}

}  // namespace RTX
}  // namespace VulkanCore