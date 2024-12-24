//
// Created by wpsimon09 on 21/12/24.
//

#ifndef SCENERENDERER_HPP
#define SCENERENDERER_HPP
#include <memory>
#include <vector>

#include "BaseRenderer.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

namespace VulkanUtils
{
    class VPushDescriptorManager;
}

namespace VulkanCore
{
    class VDevice;
}

namespace Renderer {

class RenderTarget;
class SceneRenderer: public Renderer::BaseRenderer{
public:
    SceneRenderer(const VulkanCore::VDevice& device, VulkanUtils::VPushDescriptorManager& pushDescriptorManager, int width, int height);

    void Render(int currentFrameIndex, GlobalUniform& globalUniformUpdateInfo,
        const VulkanStructs::RenderContext& renderContext, const VulkanCore::VGraphicsPipeline& pipeline) override;

protected:
    void CreateRenderTargets(VulkanCore::VSwapChain* swapChain) override;
    void RecordCommandBuffer(int currentFrameIndex, const VulkanCore::VGraphicsPipeline& pipeline) override;

private:
    VulkanUtils::VPushDescriptorManager& m_pushDescriptorManager;
    const VulkanCore::VDevice& m_device;
};

} // Renderer

#endif //SCENERENDERER_HPP
