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
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"

namespace VulkanCore::RTX {
VRayTracingBuilderKHR::VRayTracingBuilderKHR(const VulkanCore::VDevice& device)
    : m_device(device)
{
}


void VRayTracingBuilderKHR::BuildBLAS(std::vector<BLASInput>&                inputs,
                                      VulkanCore::VCommandBuffer&            cmdBuffer,
                                      VulkanCore::VTimelineSemaphore2&       frameTimeline,
                                      vk::BuildAccelerationStructureFlagsKHR flags)
{
    if(inputs.empty())
        return;
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


    VkDeviceSize hintMaxBudget{256'000'000};  // 250 MB
    bool hasCompaction = hasFlag(static_cast<VkFlags>(flags), VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    //bool hasCompaction = false;

    // scratch buffer needs to be used for every BLAS and we want ot reuse it so we will allocate scratch buffer with biggest size ever needed
    vk::DeviceSize scratchSize = blasBuilder.GetScratchSize(hintMaxBudget, asBuildData, minAlignment);
    blasScratchBuffer.CreateBufferWithAligment(scratchSize,
                                               static_cast<VkBufferUsageFlags>(vk::BufferUsageFlagBits::eShaderDeviceAddress
                                                                               | vk::BufferUsageFlagBits::eStorageBuffer),
                                               minAlignment);
    // gets the scratch buffer adress for each blasBuildData
    std::vector<vk::DeviceAddress> scratchAdresses;
    blasBuilder.GetScratchAddresses(hintMaxBudget, asBuildData, blasScratchBuffer.GetBufferAdress(), scratchAdresses, minAlignment);


    Utils::Logger::LogInfo("Building: " + std::to_string(asBuildData.size()) + "Bottom level accelerations structures");
    bool finished = false;

    m_blas.resize(asBuildData.size());
    do
    {
        {

            cmdBuffer.BeginRecording();
            finished = blasBuilder.CmdCreateParallelBlas(cmdBuffer, asBuildData, m_blas, scratchAdresses, hintMaxBudget);
            std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR};

            // submit the queue here
            vk::SemaphoreSubmitInfo signallSemaphoreInfo = ;
            cmdBuffer.EndAndFlush2(m_device.GetComputeQueue(), )

            //            cmdBuffer.EndAndFlush(m_device.GetComputeQueue(), m_asBuildSemaphore.GetSemaphore(),
            //                                  m_asBuildSemaphore.GetTimeLineSemaphoreSubmitInfo(0, 2), waitStages.data());
            // m_asBuildSemaphore.CpuWaitIdle(2);
        }
        // compact the BLAS right away


        // TODO: this has to happen after the query building as is submitted and ready, idk yet where this
        // should be, or how should i go about making this in the current architecture
        if(hasCompaction)
        {
            //          cmdBuffer.BeginRecording();
            Utils::Logger::LogInfoVerboseOnly("Compacting BLAS...");
            VulkanUtils::PlaceAccelerationStructureMemoryBarrier2(cmdBuffer.GetCommandBuffer(),
                                                                  vk::AccessFlagBits2::eAccelerationStructureReadKHR,
                                                                  vk::AccessFlagBits2::eAccelerationStructureReadKHR);
            blasBuilder.CmdCompactBlas(cmdBuffer, asBuildData, m_blas);

            //std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR};
            //          cmdBuffer.EndAndFlush(m_device.GetComputeQueue(), m_asBuildSemaphore.GetSemaphore(),
            //                                m_asBuildSemaphore.GetTimeLineSemaphoreSubmitInfo(2, 4), waitStages.data());
            //          m_asBuildSemaphore.CpuWaitIdle(4);

            blasBuilder.DestroyNonCompactedBlas();

            Utils::Logger::LogInfoVerboseOnly("BLAS compacted");
        }
    } while(!finished);

    m_device.GetTransferOpsManager().DestroyBuffer(blasScratchBuffer);
    //blasScratchBuffer.Destroy();
    blasBuilder.Destroy();
    scratchAdresses.clear();
}
void VRayTracingBuilderKHR::BuildTLAS(const std::vector<vk::AccelerationStructureInstanceKHR>& instances,
                                      VulkanCore::VCommandBuffer&                              cmdBuffer,
                                      VulkanCore::VTimelineSemaphore2&                         frameTimeline,
                                      vk::BuildAccelerationStructureFlagsKHR                   flags,
                                      bool                                                     update,
                                      bool                                                     motion)
{

    if(instances.empty())
        return;


    auto buffer = VulkanCore::VBuffer(m_device, "TLAS buffer");
    buffer.CreateBufferAndPutDataOnDevice(cmdBuffer.GetCommandBuffer(), instances,
                                          vk::BufferUsageFlagBits::eShaderDeviceAddress
                                              | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

    VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite,
                                                vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
                                                vk::AccessFlagBits2::eShaderRead};

    VulkanUtils::PlaceBufferMemoryBarrier2(cmdBuffer.GetCommandBuffer(), buffer.GetBuffer(), barrierPos);
    // acctuall creating of the TLAS
    VulkanCore::VBuffer scratchBuffer(m_device);

    CmdCreteTlas(cmdBuffer.GetCommandBuffer(), instances.size(), buffer.GetBufferAdress(), scratchBuffer, flags, update, motion);
    // std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR};
    // cmdBuffer.EndAndFlush(m_device.GetComputeQueue(), m_asBuildSemaphore.GetSemaphore(),
    //                          m_asBuildSemaphore.GetTimeLineSemaphoreSubmitInfo(0, 2), waitStages.data());

    //m_asBuildSemaphore.CpuWaitIdle(2);

    m_device.GetTransferOpsManager().DestroyBuffer(buffer);
    scratchBuffer.Destroy();
}

vk::DeviceAddress VRayTracingBuilderKHR::GetInstanceDeviceAddress(uint32_t instance) const
{
    if(instance > m_blasEntries.size())
        throw std::runtime_error("wrong instance");

    return m_blas[instance].address;
}

void VRayTracingBuilderKHR::Destroy()
{
    for(auto blas : m_blas)
    {
        blas.Destroy(m_device);
    }
    m_tlas.Destroy(m_device);
}
void VRayTracingBuilderKHR::Clear()
{
    for(auto blas : m_blas)
    {
        blas.Destroy(m_device);
    }

    m_tlas.Destroy(m_device);

    m_blas.clear();
    m_blas.shrink_to_fit();
    m_blasEntries.clear();
    m_blasEntries.shrink_to_fit();
}

void VRayTracingBuilderKHR::CmdCreteTlas(const vk::CommandBuffer&               cmdBuffer,
                                         uint32_t                               numInstances,
                                         vk::DeviceAddress                      instancesDataBuffer,
                                         VulkanCore::VBuffer&                   scratchBuffer,
                                         vk::BuildAccelerationStructureFlagsKHR flags,
                                         bool                                   update,
                                         bool                                   motion)
{

    AccelerationStructBuildData tlasBuildData;
    tlasBuildData.asType = vk::AccelerationStructureTypeKHR::eTopLevel;

    // crate geommetry info for the TLAS, since thlas does not diretly use tirangles but rather instnaces the Make instance goemetry
    // creates correct geometry info KHR and build range infos KHR;
    AccelerationStructureGeometryInfo geometryInfo = tlasBuildData.MakeInstanceGeometry(numInstances, instancesDataBuffer);
    tlasBuildData.AddGeometry(geometryInfo);

    // retrieve the scratch memory
    auto sizeInfo = tlasBuildData.FinalizeGeometry(m_device, flags);

    // allocate the scratch memory
    vk::DeviceSize scratchSize = update ? sizeInfo.updateScratchSize : sizeInfo.buildScratchSize;
    scratchBuffer.CreateBuffer(scratchSize, static_cast<VkBufferUsageFlags>(vk::BufferUsageFlagBits::eShaderDeviceAddress
                                                                            | vk::BufferUsageFlagBits::eStorageBuffer));
    vk::DeviceAddress scratchAddress = scratchBuffer.GetBufferAdress();

    // UPDATE TS
    if(update)
    {
        // we have only one instance buffer with all BLAS init
        tlasBuildData.asGeometry[0].geometry.instances.data.deviceAddress = instancesDataBuffer;
        tlasBuildData.CmdUpdateAs(cmdBuffer, m_tlas.as, scratchAddress, m_device.DispatchLoader);
    }
    else
    {

        vk::AccelerationStructureCreateInfoKHR tlasCreateInfo = tlasBuildData.DescribeCreateInfo();
#ifdef VK_NV_ray_tracing_motion_blur
        // vk::AccelerationStructureMotionInfoNV motionInfo{};
        // motionInfo.maxInstances = numInstances;


        if(motion)
        {
            //t lasCreateInfo.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_MOTION_BIT_NV;
            //tlasCreateInfo.pNext = &motionInfo;
        }
#endif
        m_tlas = AllocateAccelerationStructure(m_device, tlasCreateInfo);

        tlasBuildData.CmdBuildAs(cmdBuffer, m_tlas.as, scratchAddress, m_device.DispatchLoader);
    }
}

const vk::AccelerationStructureKHR& VRayTracingBuilderKHR::GetTLAS() const
{
    return m_tlas.as;
}
vk::AccelerationStructureKHR VRayTracingBuilderKHR::GetTLASCpy() const
{
    return m_tlas.as;
}

}  // namespace VulkanCore::RTX