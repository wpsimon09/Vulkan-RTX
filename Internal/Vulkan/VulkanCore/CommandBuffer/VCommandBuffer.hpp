//
// Created by wpsimon09 on 13/10/24.
//

#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include <mutex>

namespace VulkanCore {
class VDevice;
class VCommandPool;

class VCommandBuffer: public VObject {
public:
    explicit VCommandBuffer(const VulkanCore::VDevice& device, const VulkanCore::VCommandPool& commandPool, bool isPrimary = true);

    void Destroy() override;

    const vk::CommandBuffer& GetCommandBuffer() const {return m_commandBuffer;};
    const bool GetIsRecording()const { return m_isCurrentlyRecording; }
    const void Reset();

    void BeginRecording();
    void EndRecording();
    void EndAndFlush(const vk::Queue& queue);
    void EndAndFlush(const vk::Queue& queue, const vk::Fence& fence);
    void EndAndFlush(const vk::Queue& queue,vk::Semaphore& timeline, VkTimelineSemaphoreSubmitInfo& timelineInfo );

    void EndAndFlush(const vk::Queue& queue, std::vector<vk::Semaphore>& waitSemaphores,std::vector<vk::PipelineStageFlags>& waitStages, std::vector<vk::Semaphore>& signalSemaphores);

    ~VCommandBuffer() = default;
private:
    const VulkanCore::VDevice& m_device;
    const VulkanCore::VCommandPool& m_commandPool;
    bool m_isCurrentlyRecording;
    vk::CommandBuffer m_commandBuffer;
};

} // VulkanCore

#endif //COMMANDBUFFER_HPP
