//
// Created by wpsimon09 on 18/10/24.
//

#include "VSyncPrimitive.hpp"

#include <type_traits>
#include <vulkan/vulkan.hpp>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    template <typename T>
    VSyncPrimitive<T>::VSyncPrimitive(const VDevice &device,bool startAtInSignalState):VObject() {
        Utils::Logger::LogInfo("Creating synchronization primitive " + typeid(T).name() + " ...");
        if(std::is_same<T, vk::Semaphore>::value) {
            vk::SemaphoreCreateInfo semaphoreCreateInfo;
            m_syncPrimitive = device.GetDevice().createSemaphore(semaphoreCreateInfo);
            assert(m_syncPrimitive);
            Utils::Logger::LogSuccess("Created semaphore !");
        }
        if(std::is_same<T, vk::Fence>::value) {
            vk::FenceCreateInfo fenceCreateInfo;
            if(startAtInSignalState) {
                fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
            }
            m_syncPrimitive = device.GetDevice().createFence(fenceCreateInfo);
            assert(m_syncPrimitive);
        }
    }
} // VulkanCore
