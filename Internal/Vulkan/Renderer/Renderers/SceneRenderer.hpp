//
// Created by wpsimon09 on 21/12/24.
//

#ifndef SCENERENDERER_HPP
#define SCENERENDERER_HPP

// Standard library
#include <memory>
#include <vector>

// Project headers
#include "BaseRenderer.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"

// Forward declarations
namespace VEditor {
    class RenderingOptions;
}

namespace VulkanCore {
    class VDevice;
    class VPipelineManager;
    class VTimelineSemaphore;
    class VCommandBuffer;
    class VCommandPool;
    class VSwapChain;
}

namespace VulkanUtils {
    struct RenderContext;
    class VResourceGroupManager;
    class UIContext;
    class VRasterEffect;
    class VUniformBufferManager;
}

namespace Renderer {
    class RenderTarget;

    class SceneRenderer {
    public:
        SceneRenderer(const VulkanCore::VDevice& device,
                      ApplicationCore::EffectsLibrary& effectsLibrary,
                      VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                      int width, int height);

        void Render(int currentFrameIndex,
                    VulkanCore::VCommandBuffer& cmdBuffer,
                    const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                    VulkanUtils::RenderContext* renderContext);

        VulkanCore::VImage2& GetRenderedImage(int currentFrame)
        {
            return m_renderTargets->GetColourImage(currentFrame);
        };

        void Destroy();

    protected:
        void DrawScene(int currentFrameIndex,
                       VulkanCore::VCommandBuffer& cmdBuffer,
                       const VulkanUtils::VUniformBufferManager& uniformBufferManager);

        void DepthPrePass(int currentFrameIndex,
                          VulkanCore::VCommandBuffer& cmdBuffer,
                          const VulkanUtils::VUniformBufferManager& uniformBufferManager);

        void CreateRenderTargets(VulkanCore::VSwapChain* swapChain);

        void PushDrawCallId(const vk::CommandBuffer& cmdBuffer, VulkanStructs::VDrawCallData& drawCall );

    private:
        // Vulkan context & managers
        const VulkanCore::VDevice& m_device;
        VulkanUtils::RenderContext* m_renderContextPtr;

        // Rendering
        std::unique_ptr<Renderer::RenderTarget> m_renderTargets;
        std::shared_ptr<VulkanUtils::VEffect> m_depthPrePassEffect;

        VulkanStructs::VRenderingStatistics m_renderingStatistics;

        // Config / state
        bool m_depthPrePass = true;
        uint64_t m_frameCount = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;

        // Editor integration
        friend class VEditor::RenderingOptions;
    };

}  // namespace Renderer

#endif  // SCENERENDERER_HPP
