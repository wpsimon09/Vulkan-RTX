//
// Created by wpsimon09 on 20/04/25.
//

#include "VRayTracingDataManager.hpp"

#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "VulkanRtx.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhr.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhrHelpers.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "vulkan/vulkan.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore2.hpp"
#include <vulkan/vulkan_core.h>

namespace VulkanUtils {
VRayTracingDataManager::VRayTracingDataManager(const VulkanCore::VDevice& device)
    : m_device(device)
{
    m_rayTracingBuilder = std::make_unique<VulkanCore::RTX::VRayTracingBuilderKHR>(device);
    m_cmdPool           = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Compute);
    m_cmdBuffer         = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_cmdPool);
    m_cmdBuffer->GiveName("AS build command buffer");

    m_objDescriptionBuffer = std::make_unique<VulkanCore::VBuffer>(m_device, "All vertex and index data for RTX");
    m_objDescriptionBuffer->CreateBuffer(1, static_cast<VkBufferUsageFlags>(vk::BufferUsageFlagBits::eShaderDeviceAddress
                                                                            | vk::BufferUsageFlagBits::eStorageBuffer));
}
void VRayTracingDataManager::UpdateContext(SceneUpdateContext& sceneUpdateContext, std::vector<VulkanCore::RTX::BLASInput>& blasInputs)
{
    m_blasInputs         = blasInputs;
    m_sceneUpdateContext = &sceneUpdateContext;
}

void VRayTracingDataManager::RecordAndSubmitAsBuld(VulkanCore::VTimelineSemaphore2& frameSemaphore)
{

    if(m_sceneUpdateContext->rebuildAs || m_device.GetMeshDataManager().WasResized())
    {
        if(m_blasInputs.empty())
            return;

        frameSemaphore.CpuWaitIdle(EFrameStages::RenderFinish);

        // implicity destroys all used resources, so no cleanup of previous resources is needed
        InitAs(m_blasInputs, frameSemaphore);
        Utils::Logger::LogInfo("Rebuilding AS");

        m_sceneUpdateContext->Reset();

        return;
    }

    if(m_sceneUpdateContext->updateAs)
    {
        frameSemaphore.CpuWaitIdle(EFrameStages::RenderFinish);

        // for now every instance will be every BLAS, i will have to later redo how scene is describing the
        for(int i = 0; i < (int)m_blasInputs.size(); i++)
        {
            if(i < m_instances.size())
            {
                m_instances[i].transform = VulkanCore::RTX::GlmToMatrix4KHR(m_blasInputs[i].transform);
            }
        }
        m_rayTracingBuilder->BuildTLAS(m_instances, *m_cmdBuffer, frameSemaphore,
                                       vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace
                                           | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate,
                                       true);
        m_sceneUpdateContext->Reset();

        Utils::Logger::LogInfo("Updating AS");
    }
}


vk::DescriptorBufferInfo VRayTracingDataManager::GetObjDescriptionBufferInfo()
{
    vk::DescriptorBufferInfo info{};
    info.buffer = m_objDescriptionBuffer->GetBuffer();
    info.range  = m_objDescriptionBuffer->GetBuffeSizeInBytes();
    info.offset = 0;
    return info;
}


void VRayTracingDataManager::InitAs(std::vector<VulkanCore::RTX::BLASInput>& blasInputs, VulkanCore::VTimelineSemaphore2& frameSemaphore)
{
    if(blasInputs.empty())
        return;
    m_instances.clear();
    m_instances.shrink_to_fit();
    //m_blasInputs.clear();
    //m_blasInputs.shrink_to_fit();
    m_rtxObjectDescriptions.clear();
    m_rtxObjectDescriptions.shrink_to_fit();
    if(m_objDescriptionBuffer)
    {
        m_objDescriptionBuffer->DestroyStagingBuffer();
        m_objDescriptionBuffer->Destroy();
    }
    m_rayTracingBuilder->Clear();
    m_rayTracingBuilder->BuildBLAS(blasInputs, *m_cmdBuffer, frameSemaphore,
                                   vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace
                                       | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction);

    // single shader hit group is going to be stored under this index...
    std::vector<vk::AccelerationStructureInstanceKHR> instances;
    instances.reserve(m_blasInputs.size());

    int i                   = 0;
    int shaderHitGroupIndex = 0;
    for(auto& instance : m_blasInputs)
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
    // this causes all problems, the transfer ops manager is submitted at this point
    m_cmdBuffer->BeginRecording();
    m_objDescriptionBuffer = std::make_unique<VulkanCore::VBuffer>(m_device, "All vertex and index data for RTX");
    m_objDescriptionBuffer->CreateBufferAndPutDataOnDevice(m_cmdBuffer->GetCommandBuffer(), m_rtxObjectDescriptions,
                                                           vk::BufferUsageFlagBits::eShaderDeviceAddress
                                                               | vk::BufferUsageFlagBits::eStorageBuffer);

    m_rayTracingBuilder->BuildTLAS(m_instances, *m_cmdBuffer, frameSemaphore,
                                   vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace
                                       | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate);

    //===============================================
    // create buffer that holds vertex data adresses
}

void VRayTracingDataManager::Destroy()
{
    m_objDescriptionBuffer->DestroyStagingBuffer();
    m_objDescriptionBuffer->Destroy();
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