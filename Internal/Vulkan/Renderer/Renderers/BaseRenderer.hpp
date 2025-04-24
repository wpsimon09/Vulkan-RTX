//
// Created by wpsimon09 on 23/12/24.
//

#ifndef BASERENDERER_HPP
#define BASERENDERER_HPP
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

namespace VulkanUtils {
class UIContext;
}

namespace VulkanUtils {
class VUniformBufferManager;
}

struct GlobalUniform;

namespace VulkanCore {
class VCommandBuffer;
class VSwapChain;
class VGraphicsPipeline;
}  // namespace VulkanCore

namespace VulkanStructs {
struct RenderContext;
}

namespace VulkanCore {
class VDevice;
}

namespace Renderer {
class RenderTarget;

class BaseRenderer
{

  public:
    explicit BaseRenderer(const VulkanCore::VDevice& device);
    virtual ~BaseRenderer() = default;

    VulkanCore::VImage2& GetRenderedImage(int currentFrame)
    {
        return *m_renderTargets->m_colourAttachments[currentFrame].second;
    };  // i have to place fence to access the image
    const int&                    GetTargeWidth() const { return m_width; }
    const int&                    GetTargeHeight() const { return m_height; }
    const Renderer::RenderTarget& GetRenderTarget() const { return *m_renderTargets; }

  public:
    virtual void Destroy();

  protected:
    virtual void CreateRenderTargets(VulkanCore::VSwapChain* swapChain = nullptr)                                 = 0;

  protected:
    std::unique_ptr<Renderer::RenderTarget>                  m_renderTargets;
    const VulkanCore::VDevice&                               m_device;

    int m_width, m_height;
};
}  // namespace Renderer

#endif  //BASERENDERER_HPP
