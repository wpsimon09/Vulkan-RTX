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
        if (!m_commandBuffer->GetIsRecording())
        {
            m_commandBuffer->BeginRecording();
        }
    }

    void VTransferOperationsManager::UpdateGPU()
    {
        if (m_hasPandingWork)
        {
            m_commandBuffer->EndAndFlush(m_device.GetTransferQueue(), m_transferTimeline->GetSemaphore(), m_transferTimeline->GetSemaphoreSubmitInfo(0,2));
            m_transferTimeline->CpuWaitIdle(2);
            for (auto& buffer: m_clearValues)
            {
                vmaDestroyBuffer(m_device.GetAllocator(), buffer.first, buffer.second);
            }
        }
        m_hasPandingWork = false;
    }

    void VTransferOperationsManager::DestroyBuffer(VkBuffer& buffer, VmaAllocation & vmaAllocation)
    {
        m_clearValues.emplace_back(std::make_pair<VkBuffer, VmaAllocation>(std::move(buffer), std::move(vmaAllocation)));
    }
} // VulkanUtils