//
// Created by wpsimon09 on 13/10/24.
//

#include "VCommandBuffer.hpp"

#include "VCommandPool.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    VCommandBuffer::VCommandBuffer(const VulkanCore::VDevice &device, const VulkanCore::VCommandPool &commandPool):m_commandPool(commandPool), VObject() {
        Utils::Logger::LogInfoVerboseOnly("Creating command buffer...");
        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = m_commandPool.GetCommandBufferPool();
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        // we are only creating single command buffer here, this can be later put inside the command buffer manager that will create command buffer for each required operation
        m_commandBuffer = device.GetDevice().allocateCommandBuffers(allocInfo)[0];
        assert(m_commandBuffer);
        Utils::Logger::LogSuccess("Created command buffer that can be used with " + device.GetQueueFamilyString(commandPool.GetQueueFamily().first) + " queue");
    }

    void VCommandBuffer::Destroy() {
        //this is optional since command buffers are dealocated once command pool is destroyed
    }

    void VCommandBuffer::BeginRecording() {
        Utils::Logger::LogInfoVerboseOnly("Begin recording command buffer...");
        vk::CommandBufferBeginInfo beginInfo{};

        if(m_commandPool.GetQueueFamily().first == QUEUE_FAMILY_INDEX_TRANSFER && GlobalState::AutoCommandBufferFlags) {
            Utils::Logger::LogInfo("Command buffer is going to be used with transfer family. Setting flags to be eOneTimeSubmit, this can be changed in global state by setting variable AutoCommandBufferFlags to false!");
            beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        }
        beginInfo.pInheritanceInfo = nullptr;
        assert(m_isCurrentlyRecording == false);
        m_isCurrentlyRecording = true;
        m_commandBuffer.begin(beginInfo);
    }

    void VCommandBuffer::EndRecording() {
        assert(m_isCurrentlyRecording == true);
        m_isCurrentlyRecording = false;
        m_commandBuffer.end();
        Utils::Logger::LogInfo("Ended recording command buffer...");
    }
} // VulkanCore