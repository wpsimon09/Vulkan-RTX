//
// Created by wpsimon09 on 12/04/25.
//

#include "VRayTracingBuilderKhr.hpp"

#include "VRayTracingBlasBuilder.hpp"
#include "VRayTracingBuilderKhrHelpers.hpp"
#include "VRayTracingStructs.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/Utils/MathUtils.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

namespace VulkanCore::RTX {
VRayTracingBuilderKHR::VRayTracingBuilderKHR(const VulkanCore::VDevice& device)
    : m_device(device)
{
    m_cmdPool   = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Compute);
    m_cmdBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_cmdPool);
}


void VRayTracingBuilderKHR::BuildBLAS(std::vector<BLASInput>& inputs, vk::BuildAccelerationStructureFlagsKHR flags)
{
    VTimelineSemaphore asBuildSemaphore(m_device);
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
    uint32_t minAlignment = GlobalVariables::GlobalStructs::AccelerationStructProperties.minAccelerationStructureScratchOffsetAlignment;

    std::vector<VulkanCore::RTX::AccelerationStructBuildData> asBuildData(nbBlas);
    for(uint32_t i = 0; i < nbBlas; i++)
    {
        asBuildData[i].asType           = vk::AccelerationStructureTypeKHR::eBottomLevel;
        asBuildData[i].asGeometry       = m_blasEntries[i].input.asGeometry;
        asBuildData[i].asBuildRangeInfo = m_blasEntries[i].input.asBuildOffSetInfo;

        auto sizeInfo  = asBuildData[i].FinalizeGeometry(m_device, inputs[i].flags | flags);
        maxScratchSize = std::max(maxScratchSize, sizeInfo.buildScratchSize);
    }

    VulkanCore::VBuffer    blasScratchBuffer(m_device, "BLAS Scratch buffer");
    VRayTracingBlasBuilder blasBuilder(m_device);

vk:
    VkDeviceSize hintMaxBudget{256'000'000};  // 250 MB
   // bool hasCompaction = hasFlag(static_cast<VkFlags>(flags), VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    bool hasCompaction = false;


    // scratch buffer needs to be used for every BLAS and we want ot reuse it so we will allocate scratch buffer with biggest size ever needed
    vk::DeviceSize scratchSize = blasBuilder.GetScratchSize(hintMaxBudget, asBuildData, minAlignment);
    blasScratchBuffer.CreateBuffer(scratchSize, static_cast<VkBufferUsageFlags>(vk::BufferUsageFlagBits::eShaderDeviceAddress
                                                                                | vk::BufferUsageFlagBits::eStorageBuffer));
    // gets the scratch buffer adress for each blasBuildData
    std::vector<vk::DeviceAddress> scratchAdresses;
    blasBuilder.GetScratchAddresses(hintMaxBudget, asBuildData, blasScratchBuffer.GetBufferAdress(), scratchAdresses, minAlignment);


    Utils::Logger::LogInfo("Building: " + std::to_string(asBuildData.size()) + "Bottom level accelerations structures");
    bool finished = false;
    do
    {
        {

            m_cmdBuffer->BeginRecording();
            finished =  blasBuilder.CmdCreateParallelBlas(*m_cmdBuffer, asBuildData,m_blas, scratchAdresses, hintMaxBudget);
            std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eTopOfPipe};
            m_cmdBuffer->EndAndFlush(m_device.GetComputeQueue(), asBuildSemaphore.GetSemaphore(),
                                     asBuildSemaphore.GetSemaphoreSubmitInfo(0, 2), waitStages.data());
            asBuildSemaphore.CpuWaitIdle(2);
        }
        // compact the BLAS right away
        if (hasCompaction) {
            m_cmdBuffer->BeginRecording();
            Utils::Logger::LogInfoVerboseOnly("Compacting BLAS...");
            blasBuilder.CmdCompactBlas(*m_cmdBuffer, asBuildData, m_blas);

            std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR};
            m_cmdBuffer->EndAndFlush(m_device.GetComputeQueue(), asBuildSemaphore.GetSemaphore(),
                                     asBuildSemaphore.GetSemaphoreSubmitInfo(2, 4), waitStages.data());
            asBuildSemaphore.CpuWaitIdle(4);

            Utils::Logger::LogInfoVerboseOnly("BLAS compacted");
        }
        asBuildSemaphore.Reset();
    } while(!finished);

    asBuildSemaphore.Destroy();
    blasScratchBuffer.Destroy();
}

}  // namespace VulkanCore::RTX