//
// Created by wpsimon09 on 06/10/24.
//

#ifndef VRENDERPASS_HPP
#define VRENDERPASS_HPP

#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanCore
{
    class VDevice;
    class VSwapChain;
    class VImage;

    class VRenderPass:VObject {
    public:
        VRenderPass(const VulkanCore::VDevice& device, const VulkanCore::VSwapChain& swapChain, const VulkanCore::VImage& depthBuffer);
        void Destroy() override;

        const vk::RenderPass& GetRenderPass() const {return m_renderPass;}

    private:
        void CreateRenderPass();
        void CreateMainSubPass();
    private:
        const VulkanCore::VDevice& m_device;
        const VulkanCore::VSwapChain& m_swapChain;
        const VulkanCore::VImage& m_depthBuffer;

        vk::AttachmentDescription m_colourAttachmentDescription;
        vk::AttachmentReference m_colourAttachmentRef;

        // will be used later once i see something on the screen
        vk::AttachmentDescription m_depthStencilAttachmentDescription;
        vk::AttachmentReference m_depthStencilAttachmentRef;

        // will be used later once i see something on the screen this is part of the multisampling config
        //vk::AttachmentDescription m_resolveColourAttachmentDescription;
        //vk::AttachmentReference m_resolveColourAttachmentRef;

        vk::SubpassDescription m_subPass;
        vk::SubpassDependency m_subPassDependency;

        vk::RenderPass m_renderPass;
    };
}



#endif //VRENDERPASS_HPP
