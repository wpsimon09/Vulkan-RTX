//
// Created by wpsimon09 on 12/10/24.
//

#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.hpp>

namespace VulkanCore
{
    class VRenderPass;
    class VDevice;
    class VSwapChain;

    class VFrameBuffer:public VObject {
    public:
        VFrameBuffer(const VDevice& device, const VRenderPass& renderPass, const VSwapChain& swapChain);

        const vk::Framebuffer& GetFrameBuffer() { return m_frameBuffer.value;}

        void Destroy() override;

    private:
        const VDevice &m_device;

        vk::Framebuffer m_frameBuffer;

    };
}



#endif //FRAMEBUFFER_HPP
