//
// Created by wpsimon09 on 09/03/25.
//

#include "VTimelineSemaphore.hpp"

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    VTimelineSemaphore::VTimelineSemaphore(const VulkanCore::VDevice& device):m_device(device)
    {
        vk::SemaphoreTypeCreateInfo timelineInfo;
        timelineInfo.pNext = nullptr;
        timelineInfo.semaphoreType = vk::SemaphoreType::eTimeline;
        timelineInfo.initialValue = 0;

        vk::SemaphoreCreateInfo semaphoreInfo;
        semaphoreInfo.pNext = nullptr;
        semaphoreInfo.pNext = &timelineInfo;
        semaphoreInfo.flags = {};

        m_semaphore = m_device.GetDevice().createSemaphore(semaphoreInfo);
    }

    vk::TimelineSemaphoreSubmitInfo VTimelineSemaphore::GetSemaphoreSubmitInfo(int waitValue, int signalValue)
    {
    }

    void VTimelineSemaphore::CpuWaitIdle(int waitValue, int signalValue)
    {
    }

    void VTimelineSemaphore::Reset()
    {
    }

    uint64_t VTimelineSemaphore::GetSemaphoreValue()
    {
    }
} // VulkanCore