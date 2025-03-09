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
    explicit VTimelineSemaphore(const VulkanCore::VDevice& device);

    vk::TimelineSemaphoreSubmitInfo GetSemaphoreSubmitInfo(int waitValue, int signalValue);
    uint64_t GetSemaphoreValue();

    void CpuWaitIdle(int waitValue, int signalValue);
    void Reset();

    vk::Semaphore& GetSemaphore() {return m_semaphore;}
private:
    const VulkanCore::VDevice& m_device;

    std::vector<std::atomic_uint32_t> m_waitHistory;
    std::atomic_uint32_t m_currentWait = 0;
    std::atomic_uint32_t m_maxWait = 0; // last operation index shoudl be smaller than new operation wait on

    vk::Semaphore m_semaphore;

    uint32_t increaseValue = 2;
};

} // VulkanCore

#endif //VTIMELINESEMAPHORE_HPP
