//
// Created by wpsimon09 on 13/10/24.
//

#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"

namespace VulkanCore {
class VDevice;

class CommandBuffer: public VObject {
public:
    explicit CommandBuffer(const VulkanCore::VDevice& device);

    void Destroy() override;

    ~CommandBuffer() = default;


private:
    vk::CommandBuffer commandBuffer;
};

} // VulkanCore

#endif //COMMANDBUFFER_HPP
