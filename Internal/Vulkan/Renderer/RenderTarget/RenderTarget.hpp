//
// Created by wpsimon09 on 22/12/24.
//

#ifndef RENDERTARGET_HPP
#define RENDERTARGET_HPP
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace VulkanCore
{
    class VRenderPass;
    class VDevice;
    class VFrameBuffer;
    class VImage;
}

namespace Renderer {

class RenderTarget {
public:
    RenderTarget(const VulkanCore::VDevice& device,int width, int height, vk::Format colourFormat = vk::Format::eR8G8B8A8Unorm);

    void Finalize(int frameIndex);
private:
    std::vector<std::unique_ptr<VulkanCore::VImage>> m_colourBuffer; // for internal engine use
    std::unique_ptr<VulkanCore::VImage> m_depthBuffer; // for internal engine use
    std::vector<std::unique_ptr<VulkanCore::VFrameBuffer>> m_frameBuffers; // for passing to the Vulkan
    std::unique_ptr<std::unique_ptr<VulkanCore::VRenderPass>> m_renderPass; // puting it all together
private:
    const VulkanCore::VDevice& m_device;

};

} // Renderer

#endif //RENDERTARGET_HPP
