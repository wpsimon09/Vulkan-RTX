//
// Created by wpsimon09 on 11/03/25.
//

#include "VMA/vk_mem_alloc.h"
#include "vulkan/vulkan.h"
#include "VTransferOperationsManager.hpp"

#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

namespace VulkanUtils {
    VTransferOperationsManager::VTransferOperationsManager(const VulkanCore::VDevice& device): m_device(device)
    {
        m_commandBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, m_device.GetTransferCommandPool());
        m_transferTimeline = std::make_unique<VulkanCore::VTimelineSemaphore>(m_device);
    }

    VulkanCore::VCommandBuffer& VTransferOperationsManager::GetCommandBuffer()
    {
        m_hasPandingWork = true;
        return *m_commandBuffer;
    }

    void VTransferOperationsManager::StartRecording()
    {
        m_hasPandingWork = true;
        if (!m_commandBuffer->GetIsRecording())
        {
            m_commandBuffer->BeginRecording();
        }
    }

    void VTransferOperationsManager::UpdateGPU()
    {
        if (m_hasPandingWork)
        {
            std::vector<vk::PipelineStageFlags> waitStages = {
                vk::PipelineStageFlagBits::eVertexInput,
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eFragmentShader// once textures are here as well it will include texture as wells
            };
            m_commandBuffer->EndAndFlush(m_device.GetTransferQueue(), m_transferTimeline->GetSemaphore(), m_transferTimeline->GetSemaphoreSubmitInfo(0,2),waitStages.data());

            m_transferTimeline->CpuWaitIdle(2);

            // MOVE THIS TO SEPRATE FUNCTION THAT WILL RUN AT THE END OF EACH FRAME SO THAT RENDERER CAN RECORD COMMAND BUFFERS WITHOUT ANY ISSUES
            // THIS PREVENTS USAGE OF cpuWaitIdle AND INSTEAD GRAPHICS COMMANDS WILL GET EXECTUED
            for (auto& buffer: m_clearBuffersVKVMA)
            {
                vmaDestroyBuffer(m_device.GetAllocator(), buffer.first, buffer.second);
            }
            m_clearBuffersVKVMA.clear();
            m_hasPandingWork = false;
        }else
        {
            m_transferTimeline->CpuSignal(2);
        }
    }

    void VTransferOperationsManager::DestroyBuffer(VkBuffer& buffer, VmaAllocation & vmaAllocation)
    {
        m_clearBuffersVKVMA.emplace_back(std::make_pair<VkBuffer, VmaAllocation>(std::move(buffer), std::move(vmaAllocation)));
    }

    void VTransferOperationsManager::DestroyBuffer(VulkanCore::VBuffer& vBuffer, bool isStaging)
    {
        m_clearVBuffers.emplace_back(isStaging, &vBuffer);
    }

    void VTransferOperationsManager::Destroy()
    {
        m_transferTimeline->Destroy();
    }
} // VulkanUtils