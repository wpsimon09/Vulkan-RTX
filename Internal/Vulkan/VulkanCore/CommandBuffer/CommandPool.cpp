//
// Created by wpsimon09 on 13/10/24.
//

#include "CommandPool.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    CommandPool::CommandPool(const VulkanCore::VDevice &device, QUEUE_FAMILY_INDEX_TYPE queueFamilyType):m_device(device), VObject() {
        m_queueFamilyIndex = std::make_pair(queueFamilyType, m_device.GetConcreteQueueFamilyIndex(queueFamilyType));
        CreateCommandPool();
    }

    void CommandPool::Destroy() {
        Utils::Logger::LogInfoVerboseOnly("Destroying command pool....");
        m_device.GetDevice().destroyCommandPool(m_commandPool);
        Utils::Logger::LogInfoVerboseOnly("Command pool destroyed !");
    }

    void CommandPool::CreateCommandPool() {
        Utils::Logger::LogInfoVerboseOnly("Creating command pool...");
        vk::CommandPoolCreateInfo poolInfo = {};
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = m_queueFamilyIndex.second;
    }

} // VulkanCore