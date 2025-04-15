//
// Created by wpsimon09 on 13/04/25.
//

#include "VRayTracingBlasBuilder.hpp"

#include "VRayTracingBuilderKhrHelpers.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Utils/MathUtils.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
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
    std::vector<vk::DeviceAddress> scratchAddresses = {scratchAdress};
    return CmdCreateParallelBlas(cmdBuffer, blasBuildData, outAs, scratchAddresses, hintMaxBudget);
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
    ScratchSizeInfo sizeInfo     = CalculateScratchAlignedSize(blasBuildData, minAlignment);
    vk::DeviceSize  maxScratch   = sizeInfo.maxScratch;
    vk::DeviceSize  totalScratch = sizeInfo.totalScratch;

    if(totalScratch < hintMaxBudget)
    {
        return totalScratch;
    }
    else
    {
        uint64_t numScratch = std::max(uint64_t(1), hintMaxBudget / maxScratch);
        numScratch          = std::min(numScratch, blasBuildData.size());
        return numScratch * maxScratch;
    }
}


void VRayTracingBlasBuilder::GetScratchAddresses(vk::DeviceSize                                  hintMaxBudget,
                                                 const std::vector<AccelerationStructBuildData>& blasBuildData,
                                                 vk::DeviceAddress                               scratchBufferAderess,
                                                 std::vector<vk::DeviceAddress>&                 outScratchAddresses,
                                                 uint32_t                                        minimumAligment)
{
    ScratchSizeInfo sizeInfo     = CalculateScratchAlignedSize(blasBuildData, minimumAligment);
    vk::DeviceSize  maxScratch   = sizeInfo.maxScratch;
    vk::DeviceSize  totalScratch = sizeInfo.totalScratch;

    // in case the scratch buffer will fir every BLAS return the same thing for each BLAS build info
    if(totalScratch < hintMaxBudget)
    {
        vk::DeviceAddress address{};
        for(auto& buildData : blasBuildData)
        {
            outScratchAddresses.emplace_back(scratchBufferAderess + address);
            vk::DeviceSize alignedAdress = MathUtils::align_up(buildData.asBuildSizesInfo.buildScratchSize, minimumAligment);
            address += alignedAdress;
        }
    }
    // in case the toal size of BLAS is higher than the scratch buffer creat N * times the max scratch buffer that is fitting the budget
    else
    {
        uint64_t numScratch = std::max(uint64_t(1), hintMaxBudget / maxScratch);
        numScratch          = std::min(numScratch, blasBuildData.size());

        vk::DeviceAddress address{};
        for(int i = 0; i < numScratch; i++)
        {
            outScratchAddresses.emplace_back(scratchBufferAderess + address);
            address += maxScratch;
        }
    }
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
    if(!m_queryPool)
    {
        for(auto& blas : blasBuildData)
        {
            if(blas.hasCompactFlag())
            {
                CreateQueryPool(static_cast<uint32_t>(blasBuildData.size()));
                break;
            }
        }
    }
}

// this will actually build AS and creates buffer for it to be stored
vk::DeviceSize VRayTracingBlasBuilder::BuildAccelerationStructures(const VulkanCore::VCommandBuffer& cmdBuffer,
                                                                   std::vector<AccelerationStructBuildData>& blasBuildData,
                                                                   std::vector<AccelKHR>&                outAccel,
                                                                   const std::vector<vk::DeviceAddress>& scratchAdress,
                                                                   vk::DeviceSize                        hintMaxBudget,
                                                                   vk::DeviceSize                        currentBudget,
                                                                   uint32_t& currentQueryIndex)
{
    // tempt data to store for the build
    std::vector<vk::AccelerationStructureBuildGeometryInfoKHR> collectedBuildInfo;
    std::vector<vk::AccelerationStructureKHR>                  collectedAs;
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR*>   collectedRagneInfos;

    collectedBuildInfo.reserve(blasBuildData.size());
    collectedAs.reserve(blasBuildData.size());
    collectedRagneInfos.reserve(blasBuildData.size());

    // what is the total memory budget used by the AS build
    vk::DeviceSize totalMemoryUsed = 0;
    // acctualy build loop
    while(collectedBuildInfo.size() < scratchAdress.size() && currentBudget + totalMemoryUsed < hintMaxBudget
          && m_currentBlasIndex < blasBuildData.size())
    {
        auto& data                   = blasBuildData[m_currentBlasIndex];
        auto  createInfo             = blasBuildData[m_currentBlasIndex].DescribeCreateInfo();
        outAccel[m_currentBlasIndex] = VulkanCore::RTX::AllocateAccelerationStructure(m_device, createInfo);

        data.asBuildGoemetryInfo.mode                      = vk::BuildAccelerationStructureModeKHR::eBuild;
        data.asBuildGoemetryInfo.srcAccelerationStructure  = nullptr;
        data.asBuildGoemetryInfo.dstAccelerationStructure  = outAccel[m_currentBlasIndex].as;
        data.asBuildGoemetryInfo.scratchData.deviceAddress = scratchAdress[m_currentBlasIndex % scratchAdress.size()];
        data.asBuildGoemetryInfo.pGeometries               = data.asGeometry.data();

        collectedBuildInfo.push_back(data.asBuildGoemetryInfo);
        collectedRagneInfos.push_back(data.asBuildRangeInfo.data());

        totalMemoryUsed += data.asBuildSizesInfo.accelerationStructureSize;
        m_currentBlasIndex++;
    }

    assert(cmdBuffer.GetIsRecording() && "Command buffer is not recording, this method assumes that command buffer provided is in recording state");

    Utils::Logger::LogInfo("Build acceleration strucutres...");
    cmdBuffer.GetCommandBuffer().buildAccelerationStructuresKHR(static_cast<uint32_t>(collectedBuildInfo.size()),
                                                                collectedBuildInfo.data(), collectedRagneInfos.data(), m_device.DispatchLoader);

    // wait until all operations are completed...
    VulkanUtils::PlaceAccelerationStructureMemoryBarrier(cmdBuffer.GetCommandBuffer(), vk::AccessFlagBits::eAccelerationStructureWriteKHR,
                                      vk::AccessFlagBits::eAccelerationStructureReadKHR);

    if (m_queryPool) {
        cmdBuffer.GetCommandBuffer().writeAccelerationStructuresPropertiesKHR(static_cast<uint32_t>(collectedAs.size()), collectedAs.data(), vk::QueryType::eAccelerationStructureCompactedSizeKHR, m_queryPool, currentQueryIndex , m_device.DispatchLoader);

        currentQueryIndex += static_cast<uint32_t>(collectedAs.size());
    }

    return totalMemoryUsed;


}

}  // namespace RTX
}  // namespace VulkanCore