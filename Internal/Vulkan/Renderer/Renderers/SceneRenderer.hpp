//
// Created by wpsimon09 on 21/12/24.
//

#ifndef SCENERENDERER_HPP
#define SCENERENDERER_HPP
#include <memory>
#include <vector>

#include "BaseRenderer.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

namespace VulkanCore
{
    class VPipelineManager;
}

namespace VulkanUtils
{
    class VPushDescriptorManager;
    class UIContext;
}

namespace VulkanCore
{
    class VDevice;
}

namespace Renderer {

class RenderTarget;
class SceneRenderer: public Renderer::BaseRenderer{
public:
    SceneRenderer(const VulkanCore::VDevice& device,VulkanUtils::VPushDescriptorManager& pushDescriptorManager, int width, int height);
    void Init(const VulkanCore::VPipelineManager* pipelineManager);
    void Render(int currentFrameIndex,
                               const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                               const VulkanStructs::RenderContext& renderContext);

    void Destroy() override;
protected:
    void CreateRenderTargets(VulkanCore::VSwapChain* swapChain) override;
    void RecordCommandBuffer(int currentFrameIndex, const VulkanUtils::VUniformBufferManager& uniformBufferManager, const VulkanCore::VGraphicsPipeline& pipeline) override;
private:
    VulkanUtils::VPushDescriptorManager& m_pushDescriptorManager;
    const VulkanCore::VDevice& m_device;
    std::unique_ptr<VulkanCore::VCommandPool> m_sceneCommandPool;
    const VulkanCore::VPipelineManager* m_pipelineManager;
    const VulkanStructs::RenderContext* m_renderContextPtr;
};

} // Renderer

#endif //SCENERENDERER_HPP
