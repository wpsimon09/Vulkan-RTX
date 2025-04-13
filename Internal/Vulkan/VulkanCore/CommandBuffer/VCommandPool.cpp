//
// Created by wpsimon09 on 13/10/24.
//

#include "VCommandPool.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
VCommandPool::VCommandPool(const VulkanCore::VDevice& device, EQueueFamilyIndexType queueFamilyType)
    : m_device(device)
    , VObject()
{
  m_queueFamilyIndex = std::make_pair(queueFamilyType, m_device.GetConcreteQueueFamilyIndex(queueFamilyType));
  CreateCommandPool();
}

void VCommandPool::Destroy()
{
  Utils::Logger::LogInfoVerboseOnly("Destroying command pool....");
  m_device.GetDevice().destroyCommandPool(m_commandPool);
  Utils::Logger::LogInfoVerboseOnly("Command pool destroyed !");
}

void VCommandPool::CreateCommandPool()
{
  Utils::Logger::LogInfoVerboseOnly("Creating command pool...");
  vk::CommandPoolCreateInfo poolInfo = {};
  poolInfo.flags                     = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  poolInfo.queueFamilyIndex          = m_queueFamilyIndex.second;
  m_commandPool                      = m_device.GetDevice().createCommandPool(poolInfo);
  Utils::Logger::LogSuccess("Command pool created !");
}

}  // namespace VulkanCore