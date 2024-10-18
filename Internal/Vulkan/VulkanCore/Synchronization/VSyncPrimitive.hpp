//
// Created by wpsimon09 on 18/10/24.
//

#ifndef VSYNCPRIMITIVE_HPP
#define VSYNCPRIMITIVE_HPP
#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    class VDevice;

    template<class  T>
    class VSyncPrimitive:public VObject {
    public:
        explicit VSyncPrimitive(const VDevice& device, bool startAtInSignalState = false):m_device(device), VObject() {
            Utils::Logger::LogInfoVerboseOnly("Creating synchronization primitive...");
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

        };
        void Destroy() override {
                VObject::Destroy();
                Utils::Logger::LogInfoVerboseOnly("Destroying synchronization primitive !");
                if constexpr (std::is_same<T, vk::Semaphore>::value) {
                    m_device.GetDevice().destroySemaphore(m_syncPrimitive);
                }
                if constexpr (std::is_same<T, vk::Fence>::value) {
                    m_device.GetDevice().destroyFence(m_syncPrimitive);
                }
                Utils::Logger::LogInfoVerboseOnly("Synchronization primitive destryed !");
            };
    private:
        const VDevice& m_device;
        T m_syncPrimitive;
    };


} // VulkanCore

#endif //VSYNCPRIMITIVE_HPP
