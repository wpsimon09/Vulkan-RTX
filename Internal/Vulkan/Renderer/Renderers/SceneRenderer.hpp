//
// Created by wpsimon09 on 21/12/24.
//

#ifndef SCENERENDERER_HPP
#define SCENERENDERER_HPP
#include <memory>
#include <vector>

#include "BaseRenderer.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"

namespace VEditor {
class RenderingOptions;
}

namespace VulkanCore {
class VTimelineSemaphore;
class VPipelineManager;
}  // namespace VulkanCore

namespace VulkanUtils {
struct RenderContext;
class VResourceGroupManager;
class UIContext;
}  // namespace VulkanUtils

namespace VulkanCore {
class VDevice;
}

namespace Renderer {

class RenderTarget;
class SceneRenderer : public Renderer::BaseRenderer
{
  public:
    SceneRenderer(const VulkanCore::VDevice& device, VulkanUtils::VResourceGroupManager& pushDescriptorManager, int width, int height);
    void Render(int                                       currentFrameIndex,
                const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                VulkanUtils::RenderContext*               renderContext,
                VulkanCore::VTimelineSemaphore&           renderingTimeLine,
                VulkanCore::VTimelineSemaphore&           transferSemaphore);

    void Destroy() override;

  protected:
    void CreateRenderTargets(VulkanCore::VSwapChain* swapChain) override;
    void DrawScene(int currentFrameIndex, const VulkanUtils::VUniformBufferManager& uniformBufferManager) override;

  private:
    const VulkanCore::VDevice& m_device;

    std::unique_ptr<VulkanUtils::VEffect> m_depthPrePassEffect;

    VulkanUtils::VResourceGroupManager& m_pushDescriptorManager;

    VulkanUtils::RenderContext* m_renderContextPtr;

    std::unique_ptr<VulkanCore::VCommandPool> m_sceneCommandPool;

    // not a pointer yet
    VulkanStructs::RenderContext* m_selectedGeometryPass;

    VulkanStructs::RenderingStatistics m_renderingStatistics;

    bool m_depthPrePass = true;

    uint64_t m_frameCount = 0;

  private:
    void PushDataToGPU(const vk::CommandBuffer&                  cmdBuffer,
                       int                                       currentFrameIndex,
                       int                                       objectIndex,
                       VulkanStructs::DrawCallData&              drawCall,
                       const VulkanUtils::VUniformBufferManager& uniformBufferManager);

    void DepthPrePass(int currentFrameIndex, const VulkanUtils::VUniformBufferManager& uniformBufferManager);

    friend class VEditor::RenderingOptions;
};

}  // namespace Renderer

#endif  //SCENERENDERER_HPP
