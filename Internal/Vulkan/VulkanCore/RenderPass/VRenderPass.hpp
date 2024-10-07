//
// Created by wpsimon09 on 06/10/24.
//

#ifndef VRENDERPASS_HPP
#define VRENDERPASS_HPP

#include <vulkan/vulkan.hpp>

namespace VulkanCore
{
    class VDevice;
    class VSwapChain;

    class VRenderPass {
    public:
        VRenderPass(const VulkanCore::VDevice& device, const VulkanCore::VSwapChain& swapChain);
    private:
        const VulkanCore::VDevice& m_device;
        const VulkanCore::VSwapChain& m_swapChain;

        vk::AttachmentDescription m_colourAttachmentDescription;
        vk::AttachmentReference m_colourAttachmentRef;

        // will be used later once i see something on the screen
        //vk::AttachmentDescription m_depthStencilAttachmentDescription;
        //vk::AttachmentReference m_depthStencilAttachmentRef;

        vk::AttachmentDescription m_resolveColourAttachmentDescription;
        vk::AttachmentReference m_resolveColourAttachmentRef;

        vk::SubpassDescription m_subPass;

        vk::RenderPass m_renderPass;
    };
}



#endif //VRENDERPASS_HPP
