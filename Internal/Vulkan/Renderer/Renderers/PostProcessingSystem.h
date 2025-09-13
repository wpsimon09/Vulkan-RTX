//
// Created by wpsimon09 on 13/06/25.
//

#ifndef POSTPROCESSINGSYSTEM_H
#define POSTPROCESSINGSYSTEM_H
#include "RenderPass/PostProcessing.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/RenderPass.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"

namespace VulkanUtils {
struct RenderContext;
}
namespace Renderer {
class ToneMappingPass;
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

    void Init(int                                   frameIndex,
              VulkanUtils::VUniformBufferManager&   uniformBufferManager,
              VulkanUtils::RenderContext*           renderContext,
              VulkanStructs::PostProcessingContext* postProcessingContext);

    void Update(int                                   frameIndex,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanStructs::PostProcessingContext& postProcessingCotext);

    void Destroy();

  private:
    void ToneMapping(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext);
    void ToneMappingAverageLuminance(int                                   currentIndex,
                                     VulkanCore::VCommandBuffer&           commandBuffer,
                                     VulkanStructs::PostProcessingContext& postProcessingContext);

    void LensFlare(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext);

    void Bloom(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext);

  private:
    const VulkanCore::VDevice&          m_device;
    VulkanUtils::VUniformBufferManager& m_uniformBufferManager;

    std::unique_ptr<Renderer::ToneMappingPass> m_toneMappingPass;
    std::unique_ptr<Renderer::LensFlarePass>   m_lensFlarePass;
    std::unique_ptr<Renderer::BloomPass>       m_bloomPass;

    VulkanCore::VImage2* m_finalRender = nullptr;

    int m_width, m_height;
};

}  // namespace Renderer

#endif  //POSTPROCESSINGSYSTEM_H
