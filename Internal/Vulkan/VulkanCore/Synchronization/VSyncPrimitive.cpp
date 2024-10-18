//
// Created by wpsimon09 on 18/10/24.
//

#include "VSyncPrimitive.hpp"

#include <type_traits>
#include <vulkan/vulkan.hpp>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    template <class T>
    VSyncPrimitive<T>::VSyncPrimitive(const VDevice &device,bool startAtInSignalState):m_device(device),VObject() {
        Utils::Logger::LogInfoVerboseOnly("Creating synchronization primitive " + typeid(T).name() + " ...");
        if constexpr (std::is_same<T, vk::Semaphore>::value) {
            vk::SemaphoreCreateInfo semaphoreCreateInfo;
            m_syncPrimitive = device.GetDevice().createSemaphore(semaphoreCreateInfo);
            assert(m_syncPrimitive);
            Utils::Logger::LogSuccess("Created semaphore !");
        }
        if constexpr  (std::is_same<T, vk::Fence>::value) {
            vk::FenceCreateInfo fenceCreateInfo;
            if(startAtInSignalState) {
                fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
            }
            m_syncPrimitive = device.GetDevice().createFence(fenceCreateInfo);
            assert(m_syncPrimitive);
            Utils::Logger::LogSuccess("Created fence !");

        }
    }

    template <class T>
    void VSyncPrimitive<T>::Destroy() {
        VObject::Destroy();
        Utils::Logger::LogInfoVerboseOnly("Destroying synchronization primitive " + typeid(T).name() + " !");
        if constexpr (std::is_same<T, vk::Semaphore>::value) {
            m_device.GetDevice().destroySemaphore(m_syncPrimitive);
        }
        if constexpr (std::is_same<T, vk::Fence>::value) {
            m_device.GetDevice().destroyFence(m_syncPrimitive);
        }
        Utils::Logger::LogInfoVerboseOnly("Synchronization primitive destryed !");

    }
} // VulkanCore
