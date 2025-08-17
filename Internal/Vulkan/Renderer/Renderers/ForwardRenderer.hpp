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
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
// Forward declarations
namespace VEditor {
class RenderingOptions;
}

namespace Renderer {
class RenderTarget2;
class VisibilityBufferPass;
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
                           VulkanStructs::PostProcessingContext* postProcessingContext) {

    }

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

    void PostProcessingFogPass(int                                       currentFrameIndex,
                               VulkanCore::VCommandBuffer&               cmdBuffer,
                               const VulkanUtils::VUniformBufferManager& uniformBufferManager);

    void DenoiseVisibility(int                                       currentFrameIndex,
                           VulkanCore::VCommandBuffer&               cmdBuffer,
                           const VulkanUtils::VUniformBufferManager& uniformBufferManager);

    void PushDrawCallId(const vk::CommandBuffer& cmdBuffer, VulkanStructs::VDrawCallData& drawCall);

  private:
    // Vulkan context & managers
    const VulkanCore::VDevice&  m_device;
    VulkanUtils::RenderContext* m_renderContextPtr;

    // Rendering
    std::shared_ptr<VulkanUtils::VEffect> m_depthPrePassEffect;
    std::shared_ptr<VulkanUtils::VEffect> m_rtxShadowPassEffect;

    /**
    * Effect that will denoise the visiblity buffer  
    */
    std::unique_ptr<VulkanUtils::VComputeEffect> m_bilateralDenoiser;

    /**
         * Contains depth
         */
    std::unique_ptr<Renderer::RenderTarget2> m_depthPrePassOutput;

    /**
         *Contains world space position of the geometry
         */
    std::unique_ptr<Renderer::RenderTarget2> m_positionBufferOutput;

    std::unique_ptr<Renderer::RenderTarget2> m_normalBufferOutput;

    /**
         * Contains screen space shadow map
         */
    std::unique_ptr<Renderer::RenderTarget2> m_visibilityBuffer;
    /**
    * denoised visiblity buffer
     */
    std::unique_ptr<VulkanCore::VImage2> m_visiblityBuffer_Denoised;

    /**
    * Contains final shading of the scene
    */
    std::unique_ptr<Renderer::RenderTarget2> m_lightingPassOutput;

    /**
     * Contains the fog pass if any is required
     */
    std::unique_ptr<Renderer::RenderTarget2> m_fogPassOutput;


    VulkanCore::VImage2* m_forwardRendererOutput;

    VulkanStructs::VRenderingStatistics m_renderingStatistics;


    // Config / state
    bool     m_depthPrePass = true;
    uint64_t m_frameCount   = 0;
    uint32_t m_width        = 0;
    uint32_t m_height       = 0;

    VulkanStructs::VDrawCallData* m_postProcessingFogVolumeDrawCall = nullptr;

    std::unique_ptr<Renderer::VisibilityBufferPass> m_visibilityBufferPass;

    // Editor integration
    friend class VEditor::RenderingOptions;
};

}  // namespace Renderer

#endif  // FORWARDRENDERER_HPP
