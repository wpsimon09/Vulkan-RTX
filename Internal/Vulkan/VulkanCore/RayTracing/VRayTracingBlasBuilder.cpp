//
// Created by wpsimon09 on 13/04/25.
//

#include "VRayTracingBlasBuilder.hpp"

#include "Application/Utils/MathUtils.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
namespace RTX {
VRayTracingBlasBuilder::VRayTracingBlasBuilder(const VulkanCore::VDevice& device)
    : m_device(device)
{
}

bool VRayTracingBlasBuilder::CmdCreateBlas(const VulkanCore::VCommandBuffer&                          cmdBuffer,
                                           std::vector<VulkanCore::RTX::AccelerationStructBuildData>& blasBuildData,
                                           std::vector<VulkanCore::RTX::AccelKHR>&                    outAs,
                                           vk::DeviceAddress                                          scratchAdress,
                                           vk::DeviceSize                                             hintMaxBudget)
{
    std::vector<vk::DeviceAddress> scratchAdresses = {scratchAdress};
    return CmdCreateParallelBlas(cmdBuffer, blasBuildData, outAs, scratchAdresses, hintMaxBudget);
}

bool VRayTracingBlasBuilder::CmdCreateParallelBlas(const VulkanCore::VCommandBuffer&                          cmdBuffer,
                                                   std::vector<VulkanCore::RTX::AccelerationStructBuildData>& buildInfo,
                                                   std::vector<VulkanCore::RTX::AccelKHR>&                    outBlas,
                                                   std::vector<vk::DeviceAddress>& scratchAddresses,
                                                   vk::DeviceSize                  hintMaxBudget)
{

    InitializeQueryPoolIfNeeded(buildInfo);
    vk::DeviceSize processBudget     = 0;  // all memroy consumed during construction
    uint32_t       currentQueryIndex = m_currentQueryIndex;

    while(m_currentBlasIndex < buildInfo.size() && processBudget < hintMaxBudget)
    {
        processBudget += BuildAccelerationStructures(cmdBuffer, buildInfo, outBlas, scratchAddresses, hintMaxBudget,
                                                     processBudget, currentQueryIndex);
    }

    return m_currentBlasIndex >= buildInfo.size();
}

void VRayTracingBlasBuilder::CmdCompactBlas(const VulkanCore::VCommandBuffer&         cmdBuffer,
                                            std::vector<AccelerationStructBuildData>& blasBuildData,
                                            std::vector<AccelKHR>&                    outBlas)
{
}

void VRayTracingBlasBuilder::DestroyNonCompactedBlas() {}

VulkanCore::RTX::ScratchSizeInfo CalculateScratchAlignedSize(const std::vector<AccelerationStructBuildData>& asBuildData,
                                                             uint32_t minAlligment)

{
    vk::DeviceSize maxScratch{0};
    vk::DeviceSize totalScratch{0};

    for(auto& buildData : asBuildData)
    {
        vk::DeviceSize alignedSize = MathUtils::align_up(buildData.asBuildSizesInfo.buildScratchSize, minAlligment);
        maxScratch                 = std::max(maxScratch, alignedSize);
        totalScratch += alignedSize;
    }

    return {maxScratch, totalScratch};
}

vk::DeviceSize VRayTracingBlasBuilder::GetScratchSize(vk::DeviceSize                                  hintMaxBudget,
                                                      const std::vector<AccelerationStructBuildData>& blasBuildData,
                                                      uint32_t                                        minAlignment)
{
    ScratchSizeInfo sizeInfo = CalculateScratchAlignedSize(blasBuildData, minAlignment);
    vk::DeviceSize maxScratch = sizeInfo.maxScratch;
    vk::DeviceSize totalScratch = sizeInfo.totalScratch;

    if (totalScratch < hintMaxBudget) {
        return totalScratch;
    }else {
        uint64_t numScratch = std::max(uint64_t(1), hintMaxBudget / maxScratch);
        numScratch = std::min(numScratch, blasBuildData.size());
        return numScratch * maxScratch;
    }
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

void VRayTracingBlasBuilder::CreateQueryPool(uint32_t maxBlasCount)
{
    vk::QueryPoolCreateInfo queryPoolCI{};
    queryPoolCI.queryType  = vk::QueryType::eAccelerationStructureCompactedSizeKHR;
    queryPoolCI.queryCount = maxBlasCount;
    assert(m_device.GetDevice().createQueryPool(&queryPoolCI, nullptr, &m_queryPool) == vk::Result::eSuccess
           && "Failed to creat query pool !");
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