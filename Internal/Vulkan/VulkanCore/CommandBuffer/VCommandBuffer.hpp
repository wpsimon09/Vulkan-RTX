//
// Created by wpsimon09 on 13/10/24.
//

#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"

namespace VulkanCore {
class VDevice;
class VCommandPool;

class VCommandBuffer: public VObject {
public:
    explicit VCommandBuffer(const VulkanCore::VDevice& device, const VulkanCore::VCommandPool& commandPool);

    void Destroy() override;

    void BeginRecording();
    void EndRecording();

    ~VCommandBuffer() = default;
private:
    const VulkanCore::VCommandPool& m_commandPool;
    bool m_isCurrentlyRecording;
    vk::CommandBuffer m_commandBuffer;
};

} // VulkanCore

#endif //COMMANDBUFFER_HPP
