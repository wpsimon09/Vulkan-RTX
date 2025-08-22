//
// Created by wpsimon09 on 21/08/2025.
//

#ifndef VULKAN_RTX_POSTPROCESSING_HPP
#define VULKAN_RTX_POSTPROCESSING_HPP
#include "RenderPass.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"

namespace VulkanUtils {
class VComputeEffect;
}
namespace Renderer {

enum EFogPassAttachments
{
    FogMain = 0
};

enum EToneMappingAttachments {
    LDR =0,
    LuminanceAverage
};

class FogPass : public Renderer::RenderPass
{
  public:
    FogPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height);

    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;
    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;
    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

  private:
    FogVolumeParameters                         m_parameters;
    std::shared_ptr<VulkanUtils::VRasterEffect> m_fogPassEffect;
};

//============================================================================================================================================================

class ToneMapping : public Renderer::RenderPass
{
  public:
    ToneMapping(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height);

    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;
    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;
    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

  private:
    LuminanceHistogramParameters                 m_luminanceHistogramParameters;
    LuminanceHistogramAverageParameters          m_luminanceHistogramAverageParameters;
    ToneMappingParameters                        m_toneMappingParameters;
    std::shared_ptr<VulkanUtils::VComputeEffect> m_luminanceHistogramEffect;
    std::shared_ptr<VulkanUtils::VComputeEffect> m_luminanceAverageEffect;
    std::shared_ptr<VulkanUtils::VRasterEffect>  m_toneMappingEffect;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_POSTPROCESSING_HPP
