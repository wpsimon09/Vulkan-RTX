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
    uint32_t queryCount = m_currentBlasIndex - m_currentQueryIndex;

    if(m_queryPool == nullptr || queryCount == 0)
    {
        Utils::Logger::LogError("Query pool is null or there are no queries to get the AS size");
        return;
    }

    // get the compacted size from the query
    std::vector<vk::DeviceSize> compactedSizes(queryCount);

    assert(m_device.GetDevice().getQueryPoolResults(m_queryPool, m_currentQueryIndex,
                                                    static_cast<uint32_t>(compactedSizes.size()),
                                                    compactedSizes.size() * sizeof(vk::DeviceSize), compactedSizes.data(),
                                                    sizeof(vk::DeviceSize), vk::QueryResultFlagBits::eWait, m_device.DispatchLoader)
               == vk::Result::eSuccess
           && "Failed to retrieve size of compacted BLAS");

    // iterate over all BLASes and compact them
    // m_currentBlasIndex is the number of all BLASes
    // m_currentQueryIndex points to the query with information about the BLAs to compact
    for(size_t i = m_currentQueryIndex; i < m_currentBlasIndex; i++)
    {
        size_t         idx           = i - m_currentQueryIndex;
        vk::DeviceSize compactedSize = compactedSizes[idx];

        if(compactedSize > 0)
        {
            blasBuildData[i].asBuildSizesInfo.accelerationStructureSize = compactedSize;
            m_cleanUpdBlasAccell.push_back(outBlas[i]);  // later delete AS that is about to be compacted

            // create new AS that will be compacte
            vk::AccelerationStructureCreateInfoKHR accelCI = {};
            accelCI.size                                   = compactedSize;
            accelCI.type                                   = vk::AccelerationStructureTypeKHR::eBottomLevel;
            outBlas[i] = AllocateAccelerationStructure(m_device, accelCI);  //since this one is coppied to the clean up list, I can directly overwirte this variable with new BLAS

            vk::CopyAccelerationStructureInfoKHR copyInfo{};
            copyInfo.src  = blasBuildData[i].asBuildInfo.dstAccelerationStructure;  // set during the build of AS
            copyInfo.dst  = outBlas[i].as;
            copyInfo.mode = vk::CopyAccelerationStructureModeKHR::eCompact;

            assert(cmdBuffer.GetIsRecording() && "Command buffer is not in recording state");
            cmdBuffer.GetCommandBuffer().copyAccelerationStructureKHR(copyInfo, m_device.DispatchLoader);

            // update build data, so that it points to the correct acceleration structure and not to the one that should be deleted
            blasBuildData[i].asBuildInfo.dstAccelerationStructure = outBlas[i].as;
        }
    }
    m_currentQueryIndex = m_currentBlasIndex;
}

void VRayTracingBlasBuilder::DestroyNonCompactedBlas()
{
    for(auto& blas : m_cleanUpdBlasAccell)
    {
        blas.Destroy(m_device);
    }
    m_cleanUpdBlasAccell.clear();
}

VulkanCore::RTX::ScratchSizeInfo CalculateScratchAlignedSize(const std::vector<AccelerationStructBuildData>& asBuildData,
                                                             uint32_t minAlligment)

{
    vk::DeviceSize maxScratch{0};
    vk::DeviceSize totalScratch{0};

    for(auto& buildData : asBuildData)
    {
        //   vk::DeviceSize alignedSize = MathUtils::align_up(buildData.asBuildSizesInfo.buildScratchSize, minAlligment);
        vk::DeviceSize alignedSize = MathUtils::AlignUP(buildData.asBuildSizesInfo.buildScratchSize, minAlligment);
        assert(alignedSize == buildData.asBuildSizesInfo.buildScratchSize);

        maxScratch = std::max(maxScratch, alignedSize);
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

    // each BLAS build needs its own non-overlapping scratch adress and for this reason we have to return array of those adresses
    ScratchSizeInfo sizeInfo     = CalculateScratchAlignedSize(blasBuildData, minimumAligment);
    vk::DeviceSize  maxScratch   = sizeInfo.maxScratch;    // 733056 % 128 = 0
    vk::DeviceSize  totalScratch = sizeInfo.totalScratch;  // 4502144 % 128 = 0
    // scratch sizes are correctly aligned to 128

    // in case the scratch buffer will fir every BLAS return the same thing for each BLAS build info
    if(totalScratch < hintMaxBudget)
    {
        vk::DeviceAddress address{0};
        for(auto& buildData : blasBuildData)
        {
            outScratchAddresses.push_back(scratchBufferAderess + address);
            vk::DeviceSize alignedAdress = MathUtils::alignedSize(buildData.asBuildSizesInfo.buildScratchSize, minimumAligment);
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

void VRayTracingBlasBuilder::Destroy()
{
    if(m_queryPool)
        m_device.GetDevice().destroyQueryPool(m_queryPool);
}

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
    if(m_queryPool == nullptr)
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

    if(m_queryPool != nullptr)
    {
        m_device.GetDevice().resetQueryPool(m_queryPool, 0, static_cast<uint32_t>(blasBuildData.size()));
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
        collectedAs.push_back(outAccel[m_currentBlasIndex].as);

        data.asBuildInfo.mode                      = vk::BuildAccelerationStructureModeKHR::eBuild;
        data.asBuildInfo.srcAccelerationStructure  = nullptr;
        data.asBuildInfo.dstAccelerationStructure  = outAccel[m_currentBlasIndex].as;
        data.asBuildInfo.scratchData.deviceAddress = scratchAdress[m_currentBlasIndex % scratchAdress.size()];
        data.asBuildInfo.pGeometries               = data.asGeometry.data();

        collectedBuildInfo.push_back(data.asBuildInfo);
        collectedRagneInfos.push_back(data.asBuildRangeInfo.data());

        totalMemoryUsed += data.asBuildSizesInfo.accelerationStructureSize;
        m_currentBlasIndex++;
    }

    assert(cmdBuffer.GetIsRecording()
           && "Command buffer is not recording, this method assumes that command buffer provided is in recording state");

    Utils::Logger::LogInfo("Build acceleration strucutres...");
    cmdBuffer.GetCommandBuffer().buildAccelerationStructuresKHR(static_cast<uint32_t>(collectedBuildInfo.size()),
                                                                collectedBuildInfo.data(), collectedRagneInfos.data(),
                                                                m_device.DispatchLoader);

    // wait until all operations are completed...
    VulkanUtils::PlaceAccelerationStructureMemoryBarrier2(cmdBuffer.GetCommandBuffer(), vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
                                                          vk::AccessFlagBits2::eAccelerationStructureReadKHR);

    if(m_queryPool)
    {
        cmdBuffer.GetCommandBuffer().writeAccelerationStructuresPropertiesKHR(
            static_cast<uint32_t>(collectedAs.size()), collectedAs.data(),
            vk::QueryType::eAccelerationStructureCompactedSizeKHR, m_queryPool, currentQueryIndex, m_device.DispatchLoader);

        currentQueryIndex += static_cast<uint32_t>(collectedAs.size());
    }

    return totalMemoryUsed;
}

}  // namespace RTX
}  // namespace VulkanCore