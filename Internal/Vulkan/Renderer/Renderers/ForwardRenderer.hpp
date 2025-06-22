//
// Created by wpsimon09 on 21/12/24.
//

#ifndef FORWARDRENDERER_HPP
#define FORWARDRENDERER_HPP

// Standard library
#include <memory>
#include <vector>

// Project headers
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"

namespace Renderer {
class RenderTarget2;
}
// Forward declarations
namespace VEditor {
    class RenderingOptions;
}

namespace VulkanCore {
class VImage2;
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

    class ForwardRenderer {
    public:
        ForwardRenderer(const VulkanCore::VDevice& device,
                      ApplicationCore::EffectsLibrary& effectsLibrary,
                      VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                      int width, int height);

        void Render(int currentFrameIndex,
                    VulkanCore::VCommandBuffer& cmdBuffer,
                    const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                    VulkanUtils::RenderContext* renderContext);

        Renderer::RenderTarget2& GetDepthPrePassOutput() const;
        Renderer::RenderTarget2& GetPositionBufferOutput() const;
        Renderer::RenderTarget2& GetShadowMapOutput() const;
        Renderer::RenderTarget2& GetLightPassOutput() const;

        void Destroy();

      protected:

        void DepthPrePass(int currentFrameIndex,
                          VulkanCore::VCommandBuffer& cmdBuffer,
                          const VulkanUtils::VUniformBufferManager& uniformBufferManager);

        void ShadowMapPass(int currentFrameIndex,
                          VulkanCore::VCommandBuffer& cmdBuffer,
                          const VulkanUtils::VUniformBufferManager& uniformBufferManager);

        void DrawScene(int currentFrameIndex,
                       VulkanCore::VCommandBuffer& cmdBuffer,
                       const VulkanUtils::VUniformBufferManager& uniformBufferManager);

        void CreateRenderTargets(VulkanCore::VSwapChain* swapChain);

        void PushDrawCallId(const vk::CommandBuffer& cmdBuffer, VulkanStructs::VDrawCallData& drawCall );

    private:
        // Vulkan context & managers
        const VulkanCore::VDevice& m_device;
        VulkanUtils::RenderContext* m_renderContextPtr;

        // Rendering
        std::shared_ptr<VulkanUtils::VEffect> m_depthPrePassEffect;
        std::shared_ptr<VulkanUtils::VEffect> m_rtxShadowPassEffect;

        /**
         * Contains depth
         */
        std::unique_ptr<Renderer::RenderTarget2> m_depthPrePassOutput;

        /**
         *Contains world space position of the geometry
         */
        std::unique_ptr<Renderer::RenderTarget2> m_positionBufferOutput;

        /**
         * Contains screen space shadow map
         */
        std::unique_ptr<Renderer::RenderTarget2> m_shadowPassOutput;

        /**
         * Contains final shading of the scene
         */
        std::unique_ptr<Renderer::RenderTarget2> m_lightingPassOutput;

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

#endif  // FORWARDRENDERER_HPP
