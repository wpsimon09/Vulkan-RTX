//
// Created by wpsimon09 on 21/12/24.
//

#ifndef USERINTERFACERENDERER_HPP
#define USERINTERFACERENDERER_HPP
#include <memory>
#include <vector>

#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"


namespace VulkanCore {
class VTimelineSemaphore;
}

namespace VEditor {
class UIContext;
}

namespace VulkanCore {
class VCommandBuffer;
}


namespace VulkanCore {
class VGraphicsPipeline;
class VCommandPool;
class VSwapChain;
class VDevice;
}  // namespace VulkanCore

namespace VulkanUtils {
class VUniformBufferManager;
class UIContext;
}  // namespace VulkanUtils

namespace Renderer {
class RenderTarget;

class UserInterfaceRenderer
{
  public:
    explicit UserInterfaceRenderer(const VulkanCore::VDevice& device, const VulkanCore::VSwapChain& swapChain, VEditor::UIContext& uiContext);

    void Render(int currentFrameIndex,uint32_t swapChainImageIndex, VulkanCore::VCommandBuffer& cmdBuffer);
    void Present(uint32_t swapChainImageIndex,VulkanCore::VTimelineSemaphore& renderingTimeLine,  const vk::Semaphore& swapChainImageAvailable);

    RenderTarget& GetRenderTarget() const { return *m_renderTarget; };

    void Destroy();

  private:
    const VulkanCore::VDevice&              m_device;
    const VulkanCore::VSwapChain&           m_swapChain;
    std::unique_ptr<Renderer::RenderTarget> m_renderTarget;

    std::unique_ptr<VulkanCore::VCommandPool>                               m_commandPool;
    std::vector<std::unique_ptr<VulkanCore::VCommandBuffer>>                m_commandBuffer;
    std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_ableToPresentSemaphore;

    VEditor::UIContext& m_imguiInitializer;

  private:
    void RecordCommandBuffer(int currentFrameIndex, uint32_t swapChainImageIndex);
};
}  // namespace Renderer


#endif  //USERINTERFACERENDERER_HPP
