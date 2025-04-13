//
// Created by wpsimon09 on 13/10/24.
//

#include "VCommandBuffer.hpp"

#include "VCommandPool.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {

VCommandBuffer::VCommandBuffer(const VulkanCore::VDevice& device, const VulkanCore::VCommandPool& commandPool, bool isPrimary)
    : m_device(device)
    , m_commandPool(commandPool)
{
  Utils::Logger::LogInfoVerboseOnly(isPrimary ? "Creating primary command buffer..." : "Creating secondary command buffer...");
  vk::CommandBufferAllocateInfo allocInfo{};
  allocInfo.commandBufferCount = 1;
  allocInfo.commandPool        = m_commandPool.GetCommandBufferPool();
  allocInfo.level              = isPrimary ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary;
  // we are only creating single command buffer here, this can be later put inside the command buffer manager that will create command buffer for each required operation
  m_commandBuffer        = device.GetDevice().allocateCommandBuffers(allocInfo)[0];
  m_isCurrentlyRecording = false;
  assert(m_commandBuffer);
  Utils::Logger::LogSuccess(isPrimary ? "Creating PRIMARY " :
                                        "Created SECONDARY" + std::string("which can be used which ")
                                            + device.GetQueueFamilyString(commandPool.GetQueueFamily().first) + " queue");
}

void VCommandBuffer::Destroy()
{
  //this is optional since command buffers are deallocated once command pool is destroyed
}

const void VCommandBuffer::Reset()
{
  m_commandBuffer.reset();
  m_isCurrentlyRecording = false;
}

void VCommandBuffer::BeginRecording()
{
  Utils::Logger::LogInfoVerboseRendering("Begin recording command buffer...");
  vk::CommandBufferBeginInfo beginInfo{};

  if(m_commandPool.GetQueueFamily().first == Transfer && GlobalState::AutoCommandBufferFlags)
  {
    Utils::Logger::LogInfoVerboseOnly(
        "Command buffer is going to be used with transfer family. Setting flags to be eOneTimeSubmit, this can be changed in global state by setting variable AutoCommandBufferFlags to false!");
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  }
  beginInfo.pInheritanceInfo = nullptr;
  assert(m_isCurrentlyRecording == false);
  m_isCurrentlyRecording = true;
  m_commandBuffer.begin(beginInfo);
}


void VCommandBuffer::EndRecording()
{
  assert(m_isCurrentlyRecording == true);
  m_isCurrentlyRecording = false;
  m_commandBuffer.end();
  Utils::Logger::LogInfoVerboseRendering("Ended recording command buffer...");
}

void VCommandBuffer::EndAndFlush(const vk::Queue& queue)
{
  EndRecording();
  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &m_commandBuffer;
  assert(queue.submit(1, &submitInfo, VK_NULL_HANDLE) == vk::Result::eSuccess);
}

void VCommandBuffer::EndAndFlush(const vk::Queue& queue, const vk::Fence& fence)
{
  EndRecording();
  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &m_commandBuffer;
  assert(queue.submit(1, &submitInfo, fence) == vk::Result::eSuccess);
}

void VCommandBuffer::EndAndFlush(const vk::Queue&               queue,
                                 vk::Semaphore&                 timeline,
                                 VkTimelineSemaphoreSubmitInfo& timelineInfo,
                                 vk::PipelineStageFlags*        pWaitStages)
{
  EndRecording();
  vk::SubmitInfo submit;
  submit.pNext = &timelineInfo;

  submit.waitSemaphoreCount = 1;
  submit.pWaitSemaphores    = &timeline;

  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores    = &timeline;

  submit.commandBufferCount = 1;
  submit.pCommandBuffers    = &m_commandBuffer;

  submit.pWaitDstStageMask = pWaitStages;
  assert(queue.submit(1, &submit, nullptr) == vk::Result::eSuccess);
}


void VCommandBuffer::EndAndFlush(const vk::Queue&                     queue,
                                 std::vector<vk::Semaphore>&          waitSemaphores,
                                 std::vector<vk::PipelineStageFlags>& waitStages,
                                 std::vector<vk::Semaphore>&          signalSemaphores)
{
  std::lock_guard<std::mutex> lock(m_device.DeviceMutex);
  EndRecording();
  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &m_commandBuffer;
  submitInfo.waitSemaphoreCount = waitSemaphores.size();
  submitInfo.pWaitSemaphores    = waitSemaphores.data();
  submitInfo.pWaitDstStageMask  = waitStages.data();

  submitInfo.signalSemaphoreCount = signalSemaphores.size();
  submitInfo.pSignalSemaphores    = signalSemaphores.data();
  assert(queue.submit(1, &submitInfo, VK_NULL_HANDLE) == vk::Result::eSuccess);
}
}  // namespace VulkanCore