//
// Created by wpsimon09 on 09/03/25.
//

#include "VTimelineSemaphore.hpp"

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    VTimelineSemaphore::VTimelineSemaphore(const VulkanCore::VDevice& device, uint64_t initialValue):m_device(device)
    {
        vk::SemaphoreTypeCreateInfo timelineInfo;
        timelineInfo.pNext = nullptr;
        timelineInfo.semaphoreType = vk::SemaphoreType::eTimeline;
        timelineInfo.initialValue = initialValue;

        vk::SemaphoreCreateInfo semaphoreInfo;
        semaphoreInfo.pNext = &timelineInfo;
        semaphoreInfo.flags = {};


        m_semaphore = m_device.GetDevice().createSemaphore(semaphoreInfo);
    }

    vk::TimelineSemaphoreSubmitInfo VTimelineSemaphore::GetSemaphoreSubmitInfo(uint64_t waitValue, uint64_t signalValue)
    {
        //TODO: add validity checks for wait and signal values
        m_waitHistory.emplace_back(waitValue);
        m_currentWait = waitValue;
        m_currentSignal = signalValue;

        vk::TimelineSemaphoreSubmitInfo submitInfo;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreValueCount = 1;
        submitInfo.pWaitSemaphoreValues = &m_currentWait;
        submitInfo.signalSemaphoreValueCount = 1;
        submitInfo.pSignalSemaphoreValues = &m_currentSignal;
        return submitInfo;
    }

    void VTimelineSemaphore::CpuSignal(uint64_t signalValue)
    {
        m_currentSignal = signalValue;
        vk::SemaphoreSignalInfo signalInfo;
        signalInfo.pNext = nullptr;
        signalInfo.semaphore = m_semaphore;
        signalInfo.value = signalValue;
        m_device.GetDevice().signalSemaphore(signalInfo);
    }

    void VTimelineSemaphore::CpuWaitIdle(uint64_t waitValue)
    {
        m_currentWait = waitValue;
        vk::SemaphoreWaitInfo waitInfo;
        waitInfo.flags = {};
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &m_semaphore;
        waitInfo.pValues = &m_currentWait;

        assert(m_device.GetDevice().waitSemaphores(waitInfo, UINT64_MAX) == vk::Result::eSuccess);

    }

    void VTimelineSemaphore::Reset()
    {
        if (m_waitHistory.empty())
        {
            m_waitHistory.clear();
        }
        m_currentWait = 0;
        m_maxWait = 0;
        Destroy();
        vk::SemaphoreTypeCreateInfo timelineInfo;
        timelineInfo.pNext = nullptr;
        timelineInfo.semaphoreType = vk::SemaphoreType::eTimeline;
        timelineInfo.initialValue = 0;

        vk::SemaphoreCreateInfo semaphoreInfo;
        semaphoreInfo.pNext = &timelineInfo;
        semaphoreInfo.flags = {};


        m_semaphore = m_device.GetDevice().createSemaphore(semaphoreInfo);
    }

    void VTimelineSemaphore::Destroy()
    {
        //Reset();
        m_device.GetDevice().destroySemaphore(m_semaphore);

    }

    uint64_t VTimelineSemaphore::GetSemaphoreValue()
    {
        uint64_t waitValue = 0;
        assert(m_device.GetDevice().getSemaphoreCounterValue(m_semaphore, &waitValue) == vk::Result::eSuccess && "Failed to get semaphore value !");
        return waitValue;
    }
} // VulkanCore