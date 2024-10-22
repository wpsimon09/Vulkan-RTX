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
                    Utils::Logger::LogSuccess("Fence will start at signalled state !");

                }
                m_syncPrimitive = device.GetDevice().createFence(fenceCreateInfo);
                assert(m_syncPrimitive);
                Utils::Logger::LogSuccess("Created fence !");

            }

        };
        void Destroy() override {
                if constexpr (std::is_same<T, vk::Semaphore>::value) {
                    Utils::Logger::LogInfoVerboseOnly("Destroying semaphore !");
                    m_device.GetDevice().destroySemaphore(m_syncPrimitive);
                    Utils::Logger::LogInfoVerboseOnly("Semaphore destroyed !");
                }
                if constexpr (std::is_same<T, vk::Fence>::value) {
                    Utils::Logger::LogInfoVerboseOnly("Destroying fence !");
                    m_device.GetDevice().destroyFence(m_syncPrimitive);
                    Utils::Logger::LogInfoVerboseOnly("Destroying semaphore !");
                }
            };

        void WaitForFence() const {
            static_assert(std::is_same_v<T, vk::Fence>);
            Utils::Logger::LogInfoVerboseRendering("Waiting for fence !");
            m_device.GetDevice().waitForFences(1, &m_syncPrimitive, VK_TRUE , UINT64_MAX);
        };

        void ResetFences() const {
            static_assert(std::is_same_v<T, vk::Fence>);
            Utils::Logger::LogInfoVerboseRendering("Resetting fences !");
            m_device.GetDevice().resetFences(1, &m_syncPrimitive, VK_TRUE , UINT64_MAX);
        }

        void ResetFence() const {
            static_assert(std::is_same_v<T, vk::Fence>);
            Utils::Logger::LogInfoVerboseRendering("Resetting fence !");
            m_device.GetDevice().resetFences(1, &m_syncPrimitive);
        }

        const T& GetSyncPrimitive() const { return m_syncPrimitive; }

    private:
        const VDevice& m_device;
        T m_syncPrimitive;
    };



} // VulkanCore

#endif //VSYNCPRIMITIVE_HPP
