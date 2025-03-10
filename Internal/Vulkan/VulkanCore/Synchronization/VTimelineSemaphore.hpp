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
class VTimelineSemaphore : public VObject{
public:

    explicit VTimelineSemaphore(const VulkanCore::VDevice& device, uint64_t initialValue = 0);

    vk::TimelineSemaphoreSubmitInfo GetSemaphoreSubmitInfo(uint64_t waitValue, uint64_t signalValue);
    uint64_t GetSemaphoreValue();

    void CpuSignal(uint64_t signalValue);
    void CpuWaitIdle(uint64_t waitValue);
    void Reset();

    const uint64_t& GetCurrentWaitValue()    {return m_currentWait;};
    const uint64_t& GetCurrentSignalValue()  {return m_currentSignal;};

    vk::Semaphore& GetSemaphore() {return m_semaphore;}

    void Destroy() override;
private:
    const VulkanCore::VDevice& m_device;

    std::vector<uint64_t> m_waitHistory;
    uint64_t m_currentWait = 0;
    uint64_t m_currentSignal = 0;
    uint64_t m_maxWait = 0; // last operation index shoudl be smaller than new operation wait on

    vk::Semaphore m_semaphore;

    uint64_t increaseValue = 2;
    uint64_t m_baseSignal = 0;
    uint64_t m_baseWait = 0;
};

} // VulkanCore

#endif //VTIMELINESEMAPHORE_HPP
