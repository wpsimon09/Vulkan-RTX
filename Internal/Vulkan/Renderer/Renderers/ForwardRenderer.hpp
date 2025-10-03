//
// Created by wpsimon09 on 21/12/24.
//

#ifndef FORWARDRENDERER_HPP
#define FORWARDRENDERER_HPP

// Standard library
#include <memory>
#include <vector>

// Project headers
#include "RenderPass/VisibilityBufferPass.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
namespace Renderer {
class FogPass;
}
namespace Renderer {
class ForwardRender;
}
// Forward declarations
namespace VEditor {
class RenderingOptions;
}

namespace Renderer {
class BilateralFilterPass;
class GBufferPass;
class RenderTarget2;
class VisibilityBufferPass;
class AtmospherePass;
}  // namespace Renderer

namespace VulkanCore {
class VImage2;
class VDevice;
class VPipelineManager;
class VTimelineSemaphore;
class VCommandBuffer;
class VCommandPool;
class VSwapChain;
}  // namespace VulkanCore

namespace VulkanUtils {
class VRayTracingDataManager;
struct RenderContext;
class VResourceGroupManager;
class UIContext;
class VRasterEffect;
class VUniformBufferManager;
}  // namespace VulkanUtils

namespace Renderer {
class RenderTarget;

class ForwardRenderer
{
  public:
    ForwardRenderer(const VulkanCore::VDevice&          device,
                    VulkanUtils::RenderContext*         renderContext,
                    ApplicationCore::EffectsLibrary&    effectsLibrary,
                    VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                    int                                 width,
                    int                                 height);

    void Init(int                                  frameIndex,
              VulkanUtils::VUniformBufferManager&  uniformBufferManager,
              VulkanUtils::VRayTracingDataManager& rayTracingDataManager,
              VulkanUtils::RenderContext*          renderContext);

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::VRayTracingDataManager&  rayTracingDataManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext);

    void Render(int                                       currentFrameIndex,
                VulkanCore::VCommandBuffer&               cmdBuffer,
                const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                VulkanUtils::RenderContext*               renderContext);

    VulkanCore::VImage2*     GetForwardRendererResult() const;
    Renderer::RenderTarget2& GetDepthPrePassOutput() const;
    Renderer::RenderTarget2& GetPositionBufferOutput() const;
    Renderer::RenderTarget2& GetShadowMapOutput() const;
    Renderer::RenderTarget2& GetLightPassOutput() const;
    Renderer::RenderTarget2& GetNormalBufferOutput() const;
    VulkanCore::VImage2&     GetDenoisedVisibilityBuffer() const;

    void Destroy();

  protected:
    void DepthPrePass(int currentFrameIndex, VulkanCore::VCommandBuffer& cmdBuffer, const VulkanUtils::VUniformBufferManager& uniformBufferManager);

    void ShadowMapPass(int currentFrameIndex, VulkanCore::VCommandBuffer& cmdBuffer, const VulkanUtils::VUniformBufferManager& uniformBufferManager);

    void DrawScene(int currentFrameIndex, VulkanCore::VCommandBuffer& cmdBuffer, const VulkanUtils::VUniformBufferManager& uniformBufferManager);

    void AtmospherePass(int                                       currentFrameIndex,
                        VulkanCore::VCommandBuffer&               cmdBuffer,
                        const VulkanUtils::VUniformBufferManager& uniformBufferManager);

    void PostProcessingFogPass(int                                       currentFrameIndex,
                               VulkanCore::VCommandBuffer&               cmdBuffer,
                               const VulkanUtils::VUniformBufferManager& uniformBufferManager);

    void DenoiseVisibility(int                                       currentFrameIndex,
                           VulkanCore::VCommandBuffer&               cmdBuffer,
                           const VulkanUtils::VUniformBufferManager& uniformBufferManager);


  private:
    // Vulkan context & managers
    const VulkanCore::VDevice&  m_device;
    VulkanUtils::RenderContext* m_renderContextPtr;


    VulkanCore::VImage2* m_forwardRendererOutput;

    VulkanStructs::VRenderingStatistics m_renderingStatistics;


    // Config / state
    bool     m_depthPrePass = true;
    uint64_t m_frameCount   = 0;
    uint32_t m_width        = 0;
    uint32_t m_height       = 0;

    VulkanStructs::VDrawCallData* m_postProcessingFogVolumeDrawCall = nullptr;

    std::unique_ptr<Renderer::VisibilityBufferPass> m_visibilityBufferPass;
    std::unique_ptr<Renderer::GBufferPass>          m_gBufferPass;
    std::unique_ptr<Renderer::BilateralFilterPass>  m_visibilityDenoisePass;
    std::unique_ptr<Renderer::ForwardRender>        m_forwardRenderPass;
    std::unique_ptr<Renderer::FogPass>              m_fogPass;
    std::unique_ptr<Renderer::AtmospherePass>       m_atmospherePass;
    // Editor integration
    friend class VEditor::RenderingOptions;
};

}  // namespace Renderer

#endif  // FORWARDRENDERER_HPP
