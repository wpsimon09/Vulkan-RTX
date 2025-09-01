//
// Created by wpsimon09 on 11/03/25.
//

#include "VMA/vk_mem_alloc.h"
#include "vulkan/vulkan.h"
#include "VTransferOperationsManager.hpp"

#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

namespace VulkanUtils {
VTransferOperationsManager::VTransferOperationsManager(const VulkanCore::VDevice& device)
    : m_device(device)
{
    m_commandBuffer    = std::make_unique<VulkanCore::VCommandBuffer>(m_device, m_device.GetTransferCommandPool());
}

VulkanCore::VCommandBuffer& VTransferOperationsManager::GetCommandBuffer()
{
    m_hasPandingWork = true;
    return *m_commandBuffer;
}

void VTransferOperationsManager::StartRecording()
{
    m_hasPandingWork = true;
    if(!m_commandBuffer->GetIsRecording())
    {
        m_commandBuffer->BeginRecording();
    }
}

void VTransferOperationsManager::UpdateGPU(VulkanCore::VTimelineSemaphore& frameSemaphore)
{
    vk::PipelineStageFlags2 signalStages = vk::PipelineStageFlagBits2::eAllCommands;

    std::vector<vk::SemaphoreSubmitInfo> signalSubmit = {frameSemaphore.GetSemaphoreSignalSubmitInfo(6, signalStages)} ;

    /**
     * Submits all the transfer work, like copyes as stuff, and only signals once it is finished it does not have to wait for any other semaphore
     */
    m_commandBuffer->EndAndFlush2(m_device.GetTransferQueue(), signalSubmit, {});

    m_hasPandingWork = false;

}

void VTransferOperationsManager::UpdateGPUWaitCPU( VulkanCore::VTimelineSemaphore& frameSemaphore, bool startRecording)
{
    UpdateGPU(frameSemaphore);
    frameSemaphore.CpuWaitIdle(2);
    m_commandBuffer->Reset();

    if(startRecording)
    {
        StartRecording();
    }
}

void VTransferOperationsManager::ClearResources()
{
    //m_transferTimeline->CpuWaitIdle(2);
    for(auto& buffer : m_clearVBuffers)
    {
        if(buffer.first)  // is staging
        {
            buffer.second->DestroyStagingBuffer();
        }
        else
        {
            buffer.second->Destroy();
        }
    }

    for(auto& buffer : m_clearBuffersVKVMA)
    {
        vmaDestroyBuffer(m_device.GetAllocator(), buffer.first, buffer.second);
    }
    m_clearBuffersVKVMA.clear();
    m_clearVBuffers.clear();
}

void VTransferOperationsManager::DestroyBuffer(VkBuffer& buffer, VmaAllocation& vmaAllocation)
{
    m_clearBuffersVKVMA.emplace_back(std::make_pair<VkBuffer, VmaAllocation>(std::move(buffer), std::move(vmaAllocation)));
}

void VTransferOperationsManager::DestroyBuffer(VulkanCore::VBuffer& vBuffer, bool isStaging)
{
    m_clearVBuffers.emplace_back(isStaging, &vBuffer);
}

void VTransferOperationsManager::Destroy()
{
    ClearResources();
}
}  // namespace VulkanUtils