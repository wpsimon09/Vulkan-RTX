//
// Created by wpsimon09 on 13/06/25.
//

#ifndef POSTPROCESSINGSYSTEM_H
#define POSTPROCESSINGSYSTEM_H
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"

namespace VulkanUtils {
struct RenderContext;
}
namespace Renderer {
class ToneMapping;
}
namespace Renderer {
class RenderTarget2;
}
namespace VulkanCore {
class VImage2;
class VDevice;
}  // namespace VulkanCore
namespace Renderer {
class ForwardRenderer;

class PostProcessingSystem
{
  public:
    PostProcessingSystem(const VulkanCore::VDevice&          device,
                         ApplicationCore::EffectsLibrary&    effectsLibrary,
                         VulkanUtils::VUniformBufferManager& uniformBufferManager,
                         int                                 width,
                         int                                 height);

    void Render(int frameIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext);
    VulkanCore::VImage2& GetRenderedResult(int frameIndex);

    void Init(int                                  frameIndex,
              VulkanUtils::VUniformBufferManager&  uniformBufferManager,
              VulkanUtils::RenderContext* renderContext, VulkanStructs::PostProcessingContext* postProcessingContext);

    void Update(int frameIndex, VulkanStructs::PostProcessingContext& postProcessingCotext);

    void Destroy();

  private:
    void ToneMapping(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext);
    void ToneMappingAverageLuminance(int                                   currentIndex,
                                     VulkanCore::VCommandBuffer&           commandBuffer,
                                     VulkanStructs::PostProcessingContext& postProcessingContext);

    void LensFlare(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext);

    void AutoExposure(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext);

  private:
    std::shared_ptr<VulkanUtils::VEffect> m_toneMappingEffect;
    std::unique_ptr<RenderTarget2>        m_toneMapOutput;

    std::shared_ptr<VulkanUtils::VEffect> m_lensFlareEffect;
    std::unique_ptr<RenderTarget2>        m_lensFlareOutput;

    std::shared_ptr<VulkanUtils::VComputeEffect> m_averageLuminanceEffect;
    std::unique_ptr<VulkanCore::VImage2>         m_averageLuminanceOutput;

    std::shared_ptr<VulkanUtils::VComputeEffect> m_luminanceHistrogram;

    std::unique_ptr<Renderer::ToneMapping> m_toneMappingPass;

    const VulkanCore::VDevice&          m_device;
    VulkanUtils::VUniformBufferManager& m_uniformBufferManager;

    VulkanCore::VImage2* m_finalRender = nullptr;

    int m_width, m_height;
};

}  // namespace Renderer

#endif  //POSTPROCESSINGSYSTEM_H
