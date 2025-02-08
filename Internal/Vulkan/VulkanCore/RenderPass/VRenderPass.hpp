//
// Created by wpsimon09 on 06/10/24.
//

#ifndef VRENDERPASS_HPP
#define VRENDERPASS_HPP

#include <memory>
#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanCore
{
    class VDevice;
    class VSwapChain;
    class VImage;

    class VRenderPass:VObject {
    public:
        VRenderPass(const VulkanCore::VDevice& device, const VulkanCore::VImage& colourBuffer, const VulkanCore::VImage& depthBuffer, bool ForSwapChain = true);
        void Destroy() override;

        const vk::RenderPass& GetRenderPass() const {return m_renderPass;}
        VImage& GetMSAAImage() const {return *m_msaaImage;}

    private:
        void CreateRenderPassForSwapChain();
        void CreateRenderPassForCustomImage();
        void CreateMainSubPass();
    private:
        const VulkanCore::VDevice& m_device;
        const VulkanCore::VImage& m_depthBuffer;
        const VulkanCore::VImage& m_colourBuffer;

        std::unique_ptr<VImage> m_msaaImage;

        vk::AttachmentDescription m_colourAttachmentDescription;
        vk::AttachmentReference m_colourAttachmentRef;

        // will be used later once i see something on the screen
        vk::AttachmentDescription m_depthStencilAttachmentDescription;
        vk::AttachmentReference m_depthStencilAttachmentRef;

        vk::AttachmentDescription m_resolveColourAttachmentDescription;
        vk::AttachmentReference m_resolveColourAttachmentRef;

        vk::SubpassDescription m_subPass;
        vk::SubpassDependency m_subPassDependency;

        vk::RenderPass m_renderPass;
    };
}



#endif //VRENDERPASS_HPP
