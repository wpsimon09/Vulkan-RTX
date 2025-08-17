//
// Created by wpsimon09 on 17/08/2025.
//

#ifndef VULKAN_RTX_GBUFFERPASS_HPP
#define VULKAN_RTX_GBUFFERPASS_HPP
#include "RenderPass.hpp"

namespace Renderer {

enum EGBufferAttachments {
  Depth = 0,
  Position,
  Normal,
  Size
};

class GBufferPass : public Renderer::RenderPass
{
  public:
    GBufferPass(const VulkanCore::VDevice& device, VulkanCore::VDescriptorLayoutCache& descLayoutCache, int width, int height);

    void Init(int                                  currentFrameIndex,
              VulkanUtils::VUniformBufferManager&  uniformBufferManager,
              VulkanUtils::VRayTracingDataManager& rayTracingDataManager,
              VulkanUtils::RenderContext*          renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::VRayTracingDataManager&  rayTracingDataManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;

    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;
private:
  int m_numGBufferAttachments = EGBufferAttachments::Size;
  std::unique_ptr<VulkanUtils::VRasterEffect> m_gBufferEffect;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_GBUFFERPASS_HPP
