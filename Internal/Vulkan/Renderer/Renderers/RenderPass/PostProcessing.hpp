//
// Created by wpsimon09 on 21/08/2025.
//

#ifndef VULKAN_RTX_POSTPROCESSING_HPP
#define VULKAN_RTX_POSTPROCESSING_HPP
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "RenderPass.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace VulkanUtils {
class VComputeEffect;
}
namespace Renderer {

enum EFogPassAttachments
{
    FogMain = 0,
    FogCompute,
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

enum EBloomAttachments
{
    BloomFullRes = 0,
    A,
    B,
    C,
    D,
    E,
    BloomOutput,
    Count,
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
    FogVolumeParameters m_parameters;

    std::shared_ptr<VulkanUtils::VRasterEffect>  m_fogMergeEffect;
    std::shared_ptr<VulkanUtils::VComputeEffect> m_fogCalcEffect;
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
    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;
    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;
    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;
    void Destroy() override;

  private:
    std::shared_ptr<VulkanUtils::VComputeEffect> m_downSampleEffect;
    std::shared_ptr<VulkanUtils::VComputeEffect> m_upSampleEffect;
    std::shared_ptr<VulkanUtils::VComputeEffect> m_combineEffect;

    std::vector<vk::DescriptorImageInfo>            m_downSampleReadImages, m_downSampleWriteImages;
    std::vector<vk::DescriptorImageInfo>            m_upSampleReadImage, m_upSampleWriteImages;
    std::shared_ptr<ApplicationCore::VTextureAsset> m_lensDirtTexture = nullptr;

    BloomDownSampleParams m_downSampleParams;
    BloomUpSampleParams   m_upSampleParams;
    BloomSettings         m_bloomSettings;
};

class CompositePass : public RenderPass
{
  public:
    CompositePass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height);
    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;
    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;
    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;
    void Destroy() override;

  private:
    std::shared_ptr<VulkanUtils::VComputeEffect> m_compositeEffect;
};


}  // namespace Renderer

#endif  //VULKAN_RTX_POSTPROCESSING_HPP
