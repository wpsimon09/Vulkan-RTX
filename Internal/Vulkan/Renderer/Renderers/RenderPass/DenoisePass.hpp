//
// Created by wpsimon09 on 18/08/2025.
//

#ifndef VULKAN_RTX_DENOISEPASS_HPP
#define VULKAN_RTX_DENOISEPASS_HPP
#include "RenderPass.hpp"
#include <memory>

namespace VulkanUtils {
class VComputeEffect;
}
namespace Renderer {

enum EBilateralFilterAttachments
{
    Result = 0
};

class UpscalePass : public RenderPass
{
  public:
    UpscalePass(const VulkanCore::VDevice&       devcice,
                ApplicationCore::EffectsLibrary& effectsLibrary,
                VulkanCore::VImage2&             inputImage,
                int                              targetWidth,
                int                              targetHeight);

    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;

    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

    void Destroy() override;

  private:
    std::unique_ptr<VulkanUtils::VComputeEffect> m_upsacleEffect;
    VulkanCore::VImage2&                         m_inputImage;
};

class BilateralFilterPass : public Renderer::RenderPass
{
  public:
    /*
    Bilaterral filter denoiser, you can supply width and heigh higher to upscale the image 
    */
    BilateralFilterPass(const VulkanCore::VDevice&       device,
                        ApplicationCore::EffectsLibrary& effectsLibrary,
                        VulkanCore::VImage2&             inputImage,
                        int                              width,
                        int                              height);
    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;

    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

    void Destroy() override;

  private:
    std::unique_ptr<Renderer::UpscalePass>       m_upscalePass;
    VulkanCore::VImage2&                         m_inputImage;
    std::unique_ptr<VulkanUtils::VComputeEffect> m_bilateralFileter;
    BilaterialFilterParameters                   m_bilateralFilterParameters;
};


}  // namespace Renderer

#endif  //VULKAN_RTX_DENOISEPASS_HPP
