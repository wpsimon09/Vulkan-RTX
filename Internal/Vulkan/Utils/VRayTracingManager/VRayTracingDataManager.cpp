//
// Created by wpsimon09 on 20/04/25.
//

#include "VRayTracingDataManager.hpp"

#include "VulkanRtx.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhr.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhrHelpers.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore2.hpp"
#include "vulkan/vulkan.hpp"

namespace VulkanUtils {
VRayTracingDataManager::VRayTracingDataManager(const VulkanCore::VDevice& device)
    : m_device(device)
{
    m_cmdPool           = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Compute);

    m_cmdBuffer.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
        m_cmdBuffer[i] = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_cmdPool);
    }
    m_rayTracingBuilder = std::make_unique<VulkanCore::RTX::VRayTracingBuilderKHR>(device);

    m_objDescriptionBuffer.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
}

void VRayTracingDataManager::UpdateAS(std::vector<VulkanCore::RTX::BLASInput>& blasInputs, VulkanCore::VTimelineSemaphore2& frameTimeline)
{
    // for now every instance will be every BLAS, i will have to later redo how scene is describing the
    for(int i = 0; i < (int)blasInputs.size(); i++)
    {
        m_instances[i].transform = VulkanCore::RTX::GlmToMatrix4KHR(blasInputs[i].transform);
    }
    m_rayTracingBuilder->BuildTLAS(m_instances, *m_cmdBuffer[m_device.CurrentFrameInFlight], frameTimeline,
                                   vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace
                                       | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate,
                                   true);
}


vk::DescriptorBufferInfo VRayTracingDataManager::GetObjDescriptionBufferInfo()
{
    vk::DescriptorBufferInfo info{};
    info.buffer = m_objDescriptionBuffer[m_device.CurrentFrameInFlight]->GetBuffer();
    info.range  = vk::WholeSize;
    return info;
}

void VRayTracingDataManager::Update(ApplicationCore::Scene& scene, VulkanCore::VTimelineSemaphore2& frameTimeline)
{
    // As build is single buffered, therefore I have to wait until cmd buffer is done using it.
    if (m_device.CurrentFrame > 0) {
        frameTimeline.CpuWaitIdle(EFrameStages::AsBuildFinish);
    }

    if (!scene.GetSceneUpdateFlags().updateAs && !scene.GetSceneUpdateFlags().rebuildAs) {
        frameTimeline.CpuSignal(EFrameStages::AsBuildFinish);
    }

    // wait until the semaphore signals `AccelerationStructureUpdateStage`
    // if the frame  0, do not wait for signal
    // if the as build does not update, just signal through cpu at the end of the frame
    // TODO: figure out when should I submit the recorded async compute command buffer .
    if(scene.GetSceneUpdateFlags().rebuildAs)
    {
        auto blasInpu = scene.GetBLASInputs();
        if(blasInpu.empty())
            return;

        // implicity destroys all used resources, so no cleanup of previous resources is needed
        InitAs(blasInpu, frameTimeline);
        Utils::Logger::LogInfo("Rebuilding AS");

        // TODO: this is hacky fix and not according to the standart, maybe callback function could fix this
        // this happens because i am reseting the rebuildAS because it is being flagged in Render() and Update() is before render
    }

    if(scene.GetSceneUpdateFlags().updateAs)
    {
        auto blasInput = scene.GetBLASInputs();
        if(blasInput.empty())
            return;
        UpdateAS(blasInput, frameTimeline);
        Utils::Logger::LogInfo("Updating AS");
    }
}

void VRayTracingDataManager::InitAs(std::vector<VulkanCore::RTX::BLASInput>& blasInputs, VulkanCore::VTimelineSemaphore2& frameTimeline)
{
    if(blasInputs.empty())
        return;
    m_instances.clear();
    m_instances.shrink_to_fit();
    m_blasInputs.clear();
    m_blasInputs.shrink_to_fit();
    m_rtxObjectDescriptions.clear();
    m_rtxObjectDescriptions.shrink_to_fit();
    if(m_objDescriptionBuffer[m_device.CurrentFrameInFlight])
    {
        m_objDescriptionBuffer[m_device.CurrentFrameInFlight]->DestroyStagingBuffer();
        m_objDescriptionBuffer[m_device.CurrentFrameInFlight]->Destroy();
    }
    m_rayTracingBuilder->Clear();
    m_rayTracingBuilder->BuildBLAS(blasInputs, *m_cmdBuffer[m_device.CurrentFrameInFlight], frameTimeline,
                                   vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace
                                       | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction);

    // single shader hit group is going to be stored under this index...
    std::vector<vk::AccelerationStructureInstanceKHR> instances;
    instances.reserve(blasInputs.size());

    int i                   = 0;
    int shaderHitGroupIndex = 0;
    for(auto& instance : blasInputs)
    {

        vk::AccelerationStructureInstanceKHR instanceInfo{};
        instanceInfo.transform           = VulkanCore::RTX::GlmToMatrix4KHR(instance.transform);
        instanceInfo.instanceCustomIndex = i;

        instanceInfo.flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instanceInfo.accelerationStructureReference         = m_rayTracingBuilder->GetInstanceDeviceAddress(i);
        instanceInfo.mask                                   = 0xFF;
        instanceInfo.instanceShaderBindingTableRecordOffset = 0;

        m_instances.emplace_back(instanceInfo);
        m_rtxObjectDescriptions.emplace_back(blasInputs[i].objDescription);
        i++;
    }
    m_rayTracingBuilder->BuildTLAS(m_instances, *m_cmdBuffer[m_device.CurrentFrameInFlight], frameTimeline,
                                   vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace
                                       | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate);

    //===============================================
    // create buffer that holds vertex data adresses
    m_objDescriptionBuffer[m_device.CurrentFrameInFlight] = std::make_unique<VulkanCore::VBuffer>(m_device, "All vertex and index data for RTX");
    m_objDescriptionBuffer[m_device.CurrentFrameInFlight]->CreateBufferAndPutDataOnDevice(
        m_device.GetTransferOpsManager().GetCommandBuffer().GetCommandBuffer(), m_rtxObjectDescriptions,
        vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);

    //=================================================
    // Here submit the work from tlas build
}

void VRayTracingDataManager::SubmitGPUWork(VulkanCore::VTimelineSemaphore2& frameTimeline) {

    vk::SemaphoreSubmitInfo signalInfo = frameTimeline.GetSemaphoreSignalSubmitInfo(EFrameStages::AsBuildFinish, {});
    vk::SemaphoreSubmitInfo waitInfo = frameTimeline.GetSemaphoreWaitSubmitInfo(EFrameStages::TransferFinish, vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR);

    m_cmdBuffer[m_device.CurrentFrameInFlight]->EndAndFlush2(m_device.GetComputeQueue(), signalInfo, waitInfo);
}

void VRayTracingDataManager::Destroy()
{
    m_objDescriptionBuffer[m_device.CurrentFrameInFlight]->DestroyStagingBuffer();
    m_objDescriptionBuffer[m_device.CurrentFrameInFlight]->Destroy();
    m_rayTracingBuilder->Destroy();
    m_cmdPool->Destroy();
}

const vk::AccelerationStructureKHR& VRayTracingDataManager::GetTLAS()
{
    return m_rayTracingBuilder->GetTLAS();
}
vk::AccelerationStructureKHR VRayTracingDataManager::GetTLASCpy()
{
    return m_rayTracingBuilder->GetTLASCpy();
}

}  // namespace VulkanUtils