//
// Created by wpsimon09 on 09/03/25.
//

#ifndef VTIMELINESEMAPHORE_HPP
#define VTIMELINESEMAPHORE_HPP
#include <atomic>
#include <vector>

#include "Vulkan/VulkanCore/VObject.hpp"
#include "vulkan/vulkan.hpp"

namespace VulkanCore {
class VDevice;
class VTimelineSemaphore : public VObject
{
  public:
    explicit VTimelineSemaphore(const VulkanCore::VDevice& device, uint64_t initialValue = 0);

    vk::TimelineSemaphoreSubmitInfo GetTimeLineSemaphoreSubmitInfo(uint64_t waitValue, uint64_t signalValue);
    vk::SemaphoreSubmitInfo         GetSemaphoreWaitSubmitInfo(uint64_t waitValue, vk::PipelineStageFlags2 waitStages);
    vk::SemaphoreSubmitInfo GetSemaphoreSignalSubmitInfo(uint64_t signalValue, vk::PipelineStageFlags2 signalStages);
    uint64_t                GetSemaphoreValue();

    void CpuSignal(uint64_t signalValue);
    void CpuWaitIdle(uint64_t waitValue);
    void SetWaitAndSignal(uint64_t waitValue, uint64_t signalValue);
    void Reset();

    const uint64_t& GetCurrentWaitValue() const { return m_currentWait; };
    const uint64_t& GetCurrentSignalValue() const { return m_currentSignal; };
    const uint64_t& GetOffset() const { return m_offset; };

    vk::Semaphore& GetSemaphore() { return m_semaphore; }

    void Destroy() override;

  private:
    const VulkanCore::VDevice& m_device;

    std::vector<uint64_t> m_waitHistory;
    uint64_t              m_currentWait   = 0;
    uint64_t              m_currentSignal = 0;

    vk::Semaphore m_semaphore;

    //uint64_t m_currentValue = 0;

    uint64_t m_offset = 0;
};

}  // namespace VulkanCore

#endif  //VTIMELINESEMAPHORE_HPP
