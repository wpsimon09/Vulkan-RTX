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

enum EToneMappingAttachments
{
    LDR = 0,
    LuminanceAverage
};

enum ELensFlareAttachments
{
    LensFlareMain = 0
};

//============================================================================================================================================================

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

    void Destroy() override;

  private:
    FogVolumeParameters                         m_parameters;
    std::shared_ptr<VulkanUtils::VRasterEffect> m_fogPassEffect;
};

//============================================================================================================================================================

class ToneMappingPass : public Renderer::RenderPass
{
  public:
    ToneMappingPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height);

    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;
    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;
    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

    void Destroy() override;

  private:
    LuminanceHistogramParameters        m_luminanceHistogramParameters;
    LuminanceHistogramAverageParameters m_averageLuminanceParameters;
    ToneMappingParameters               m_toneMappingParameters;

    std::shared_ptr<VulkanUtils::VComputeEffect> m_luminanceHistogramEffect;
    std::shared_ptr<VulkanUtils::VComputeEffect> m_averageLuminanceEffect;
    std::shared_ptr<VulkanUtils::VRasterEffect>  m_toneMappingEffect;
    std::vector<vk::Buffer>                      m_luminanceHistogramBuffer;
};

//============================================================================================================================================================

class LensFlarePass : public RenderPass
{
  public:
    LensFlarePass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height);
    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;
    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;
    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;
    void Destroy() override;

  private:
    LensFlareParameters                         m_lensFlareParameters;
    std::shared_ptr<VulkanUtils::VRasterEffect> m_lensFlareEffect;
};

class BloomPass : public RenderPass
{
  public:
    BloomPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height);

  private:
};

}  // namespace Renderer

#endif  //VULKAN_RTX_POSTPROCESSING_HPP
