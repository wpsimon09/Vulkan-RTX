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

    ~RenderTarget() = default;
private:

    std::vector<std::unique_ptr<VulkanCore::VImage>> m_colourBuffer; // for internal engine use
    std::unique_ptr<VulkanCore::VImage> m_depthBuffer; // for internal engine use
    std::unique_ptr<VulkanCore::VImage> m_msaaBuffer; // for internal engine use

    std::vector<std::unique_ptr<VulkanCore::VFrameBuffer>> m_frameBuffers; // for passing to the Vulkan
    std::unique_ptr<VulkanCore::VRenderPass> m_renderPass; // putting it all together
private:
    const VulkanCore::VDevice& m_device;
    int m_width, m_height;

    friend Renderer::BaseRenderer;
    friend Renderer::UserInterfaceRenderer;

    void CreateRenderTargetForSwapChain(const VulkanCore::VSwapChain& swapChain);
};

} // Renderer

#endif //RENDERTARGET_HPP
