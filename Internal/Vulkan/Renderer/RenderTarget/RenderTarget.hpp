//
// Created by wpsimon09 on 22/12/24.
//

#ifndef RENDERTARGET_HPP
#define RENDERTARGET_HPP
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Renderer
{
    class UserInterfaceRenderer;
}

namespace Renderer
{
    class BaseRenderer;
}

namespace VulkanCore
{
    class VImage2;
    class VSwapChain;
    class VRenderPass;
    class VDevice;
    class VFrameBuffer;
    class VImage;
}

namespace Renderer {

class RenderTarget {
public:
    RenderTarget(const VulkanCore::VDevice& device,int width, int height, vk::Format colourFormat = vk::Format::eR8G8B8A8Unorm);
    RenderTarget(const VulkanCore::VDevice& device, const VulkanCore::VSwapChain& swapChain);

    void HandleResize(int newWidth, int newHeight);
    void HandleSwapChainResize(const VulkanCore::VSwapChain& swapChain);
    void Destroy();
    void DestroyForResize();

    vk::ImageView GetColourImageView(int currentFrame) const;
    vk::ImageView GetDepthImageView() const;
    vk::ImageView GetResolveImageView(int currentFrame) const;

    vk::RenderingAttachmentInfo& GetColourAttachmentOneSample(int currentFrame) ;
    /**
     * This is the attachment that will be used to render into, use to to specify the render target for the render pass !!!
     * @param currentFrame current index of the frame 0 / 1
     * @return attachment info
     */
    vk::RenderingAttachmentInfo& GetColourAttachmentMultiSampled(int currentFrame) ;

    vk::RenderingAttachmentInfo& GetDepthAttachment() ;

    const VulkanCore::VImage2& GetColourImage(int currentFrame) const;
    const VulkanCore::VImage2& GetDepthImage(int currentFrame) const;
    const VulkanCore::VImage2& GetColourAttachmentMultiSampled(int currentFrame) const;

    ~RenderTarget() = default;
private:

    std::vector<std::pair<vk::RenderingAttachmentInfo, std::unique_ptr<VulkanCore::VImage2>>> m_colourAttachments; // for internal engine use
    std::pair<vk::RenderingAttachmentInfo, std::unique_ptr<VulkanCore::VImage2>> m_depthAttachment; // for internal engine use
    std::vector<std::pair<vk::RenderingAttachmentInfo, std::unique_ptr<VulkanCore::VImage2>>> m_msaaAttachments; // for internal engine use

private:
    const VulkanCore::VDevice& m_device;
    int m_width, m_height;
    bool m_forSwapChain = false;

    friend Renderer::BaseRenderer;
    friend Renderer::UserInterfaceRenderer;

    void CreateRenderTargetForSwapChain(const VulkanCore::VSwapChain& swapChain);
};

} // Renderer

#endif //RENDERTARGET_HPP
