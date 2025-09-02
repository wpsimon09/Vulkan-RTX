//
// Created by wpsimon09 on 02/09/2025.
//

#include "VTimelineSemaphore2.hpp"

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
VTimelineSemaphore2::VTimelineSemaphore2(const VulkanCore::VDevice& device, uint32_t maxStageValue)
    : VObject()
    , m_device(device)
    , m_maxStageValue(maxStageValue)
{
    vk::SemaphoreTypeCreateInfo timelineInfo;
    timelineInfo.pNext         = nullptr;
    timelineInfo.semaphoreType = vk::SemaphoreType::eTimeline;
    timelineInfo.initialValue  = 0;

    vk::SemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.pNext = &timelineInfo;
    semaphoreInfo.flags = {};

    m_semaphore = m_device.GetDevice().createSemaphore(semaphoreInfo);
}
vk::SemaphoreSubmitInfo VTimelineSemaphore2::GetSemaphoreWaitSubmitInfo(uint32_t stage, vk::PipelineStageFlags2 waitStages)
{
    vk::SemaphoreSubmitInfo waitSubmitInfo{};
    waitSubmitInfo.semaphore = m_semaphore;
    waitSubmitInfo.value     = GetStageValue(stage);
    waitSubmitInfo.stageMask = waitStages;

    return waitSubmitInfo;
}

vk::SemaphoreSubmitInfo VTimelineSemaphore2::GetSemaphoreSignalSubmitInfo(uint32_t stage, vk::PipelineStageFlags2 signalStages)
{
    vk::SemaphoreSubmitInfo signalSubmitInfo{};
    signalSubmitInfo.semaphore = m_semaphore;
    signalSubmitInfo.value     = GetStageValue(stage);
    signalSubmitInfo.stageMask = signalStages;

    return signalSubmitInfo;
}
void VTimelineSemaphore2::CpuSignal(uint32_t signalStage)
{
    vk::SemaphoreSignalInfo signalInfo;
    signalInfo.pNext     = nullptr;
    signalInfo.semaphore = m_semaphore;
    signalInfo.value     = GetStageValue(signalStage);
    m_device.GetDevice().signalSemaphore(signalInfo);
}

void VTimelineSemaphore2::CpuWaitIdle(uint32_t waitStage)
{
    const uint64_t value = GetStageValue(waitStage);

    vk::SemaphoreWaitInfo waitInfo;
    waitInfo.flags          = {};
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores    = &m_semaphore;
    waitInfo.pValues        = &value;

    while(m_device.GetDevice().waitSemaphores(waitInfo, UINT64_MAX) == vk::Result::eTimeout)
        ;
}


uint32_t VTimelineSemaphore2::GetStageValue(uint32_t stage) const
{
    return (m_device.CurrentFrame * m_maxStageValue) + stage;
}


void VTimelineSemaphore2::Destroy()
{
    m_device.GetDevice().destroySemaphore(m_semaphore);
}
}  // namespace VulkanCore