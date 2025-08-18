//
// Created by wpsimon09 on 18/08/2025.
//

#ifndef VULKAN_RTX_DENOISEPASS_HPP
#define VULKAN_RTX_DENOISEPASS_HPP
#include "RenderPass.hpp"

namespace VulkanUtils {
class VComputeEffect;
}
namespace Renderer {

enum EBilateralFilterAttachments {
  Result = 0
};

class BilateralFilterPass: public Renderer::RenderPass
{
  public:
    BilateralFilterPass(const VulkanCore::VDevice& device,VulkanCore::VDescriptorLayoutCache& descLayoutCache,  VulkanCore::VImage2& inputImage, int width, int height);
    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;

    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

    void SetImageToDenoise(VulkanCore::VImage2* imageToDenoise);
private:
  VulkanCore::VImage2& m_inputImage;
  std::unique_ptr<VulkanUtils::VComputeEffect> m_bilateralFileter;
  BilaterialFilterParameters m_bilateralFilterParameters;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_DENOISEPASS_HPP
