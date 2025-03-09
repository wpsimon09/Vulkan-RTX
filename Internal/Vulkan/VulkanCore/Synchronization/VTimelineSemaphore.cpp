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

    vk::TimelineSemaphoreSubmitInfo VTimelineSemaphore::GetSemaphoreSubmitInfo(uint64_t waitValue, uint64_t signalValue)
    {
        //TODO: add validity checks for wait and signal values
        m_waitHistory.emplace_back(waitValue);

        vk::TimelineSemaphoreSubmitInfo submitInfo;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreValueCount = 1;
        submitInfo.pWaitSemaphoreValues = &m_waitHistory.back();
        submitInfo.signalSemaphoreValueCount = 1;
        submitInfo.pSignalSemaphoreValues = &signalValue;
        return submitInfo;
    }

    void VTimelineSemaphore::CpuWaitIdle(uint64_t signalValue)
    {
        vk::SemaphoreSignalInfo signalInfo;
        signalInfo.pNext = nullptr;
        signalInfo.semaphore = m_semaphore;
        signalInfo.value = signalValue;
        m_device.GetDevice().signalSemaphore(signalInfo);
    }

    void VTimelineSemaphore::Reset()
    {
        m_waitHistory.clear();
        m_currentWait = 0;
        m_maxWait = 0;
    }

    void VTimelineSemaphore::Destroy()
    {
        Reset();
        m_device.GetDevice().destroySemaphore(m_semaphore);
    }

    uint64_t VTimelineSemaphore::GetSemaphoreValue()
    {
        uint64_t waitValue = 0;
        assert(m_device.GetDevice().getSemaphoreCounterValue(m_semaphore, &waitValue) == vk::Result::eSuccess && );
        return waitValue;
    }
} // VulkanCore