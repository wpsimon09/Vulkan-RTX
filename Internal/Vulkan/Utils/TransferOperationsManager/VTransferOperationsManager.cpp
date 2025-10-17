//
// Created by wpsimon09 on 11/03/25.
//

#include "VTransferOperationsManager.hpp"
#include "VMA/vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore2.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace VulkanUtils {
VTransferOperationsManager::VTransferOperationsManager(const VulkanCore::VDevice& device)
    : m_device(device)
{
    m_commandBuffer.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_commandBuffer[i] = std::make_unique<VulkanCore::VCommandBuffer>(m_device, m_device.GetTransferCommandPool());
        auto cmdBufferName = "Transfer command buffer | frame index: " + std::to_string(i);
        m_commandBuffer[i]->GiveName(cmdBufferName);
    }
    m_clearBuffersVKVMA.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_clearImages.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_clearVBuffers.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
}

VulkanCore::VCommandBuffer& VTransferOperationsManager::GetCommandBuffer()
{
    m_hasPandingWork = true;
    return *m_commandBuffer[m_device.CurrentFrameInFlight];
}

void VTransferOperationsManager::StartRecording()
{
    m_hasPandingWork = true;

    m_commandBuffer[m_device.CurrentFrameInFlight]->BeginRecording();
}

void VTransferOperationsManager::UpdateGPU(VulkanCore::VTimelineSemaphore2& frameSemaphore)
{
    vk::PipelineStageFlags2 signalStages = vk::PipelineStageFlagBits2::eAllCommands;

    std::vector<vk::SemaphoreSubmitInfo> signalSubmit = {
        frameSemaphore.GetSemaphoreSignalSubmitInfo(EFrameStages::TransferFinish, signalStages)};

    /**
     * Submits all the transfer work, like copyes as stuff, and only signals once it is finished it does not have to wait for any other semaphore
     */
    m_commandBuffer[m_device.CurrentFrameInFlight]->EndAndFlush2(m_device.GetTransferQueue(), signalSubmit, {});

    m_hasPandingWork = false;
}

void VTransferOperationsManager::UpdateGPUWaitCPU(VulkanCore::VTimelineSemaphore& frameSemaphore, bool startRecording)
{
    //pdateGPU(frameSemaphore);
    //frameSemaphore.CpuWaitIdle(9);
    //frameSemaphore.Reset();
    //frameSemaphore.CpuSignal(8);
    m_commandBuffer[m_device.CurrentFrameInFlight]->Reset();

    if(startRecording)
    {
        StartRecording();
    }
}

void VTransferOperationsManager::ClearResources()
{
    //m_transferTimeline->CpuWaitIdle(2);
    for(auto& buffer : m_clearVBuffers[m_device.CurrentFrameInFlight])
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

    for(auto& buffer : m_clearBuffersVKVMA[m_device.CurrentFrameInFlight])
    {
        vmaDestroyBuffer(m_device.GetAllocator(), buffer.first, buffer.second);
    }
    for(auto& image : m_clearImages[m_device.CurrentFrameInFlight])
    {
        vmaDestroyImage(m_device.GetAllocator(), image.first, image.second);
    }

    m_clearImages[m_device.CurrentFrameInFlight].clear();
    m_clearBuffersVKVMA[m_device.CurrentFrameInFlight].clear();
    m_clearVBuffers[m_device.CurrentFrameInFlight].clear();
}

void VTransferOperationsManager::DestroyBuffer(VkBuffer& buffer, VmaAllocation& vmaAllocation)
{
    m_clearBuffersVKVMA[m_device.CurrentFrameInFlight].emplace_back(
        std::make_pair<VkBuffer, VmaAllocation>(std::move(buffer), std::move(vmaAllocation)));
}

void VTransferOperationsManager::DestroyBuffer(VulkanCore::VBuffer& vBuffer, bool isStaging)
{
    m_clearVBuffers[m_device.CurrentFrameInFlight].emplace_back(isStaging, &vBuffer);
}

void VTransferOperationsManager::DestroyImage(vk::Image image, VmaAllocation& vmaAllocation)
{
    m_clearImages[m_device.CurrentFrameInFlight].emplace_back(image, vmaAllocation);
}

void VTransferOperationsManager::AddToSyncList(std::shared_ptr<ApplicationCore::VTextureAsset> texture)
{
    m_texturesToSync.emplace_back(texture);
}

void VTransferOperationsManager::Sync()
{
    int i = 0;
    if(!m_texturesToSync.empty())
    {
        for(auto& tex : m_texturesToSync)
        {
            if(tex)
            {
                if(tex->Sync())
                {
                    m_texturesToSync.erase(m_texturesToSync.begin() + (i));
                    //m_texturesToSync.shrink_to_fit();
                }
            }
            i++;
        }
    }
}


void VTransferOperationsManager::Destroy()
{
    ClearResources();
}
}  // namespace VulkanUtils