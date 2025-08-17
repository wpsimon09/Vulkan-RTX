//
// Created by wpsimon09 on 16/08/2025.
//

#ifndef VULKAN_RTX_RTSHADOWMAPPASS_HPP
#define VULKAN_RTX_RTSHADOWMAPPASS_HPP
#include "RenderPass.hpp"

namespace VulkanUtils {
struct RenderContext;
}
namespace Renderer {


class VisibilityBufferPass : public Renderer::RenderPass
{
  public:
    VisibilityBufferPass(const VulkanCore::VDevice& device, VulkanCore::VDescriptorLayoutCache& descLayoutCache, int width, int height);

    void Init(int frameIndex,
              VulkanUtils::VUniformBufferManager&  uniformBufferManager,
              VulkanUtils::VRayTracingDataManager& rayTracingDataManager,
              VulkanUtils::RenderContext*          renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::VRayTracingDataManager&  rayTracingDataManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext ) override;

    void Render(int currentFrame, const vk::CommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;


  private:
    std::unique_ptr<VulkanUtils::VRasterEffect> m_rayTracedShadowEffect;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_RTSHADOWMAPPASS_HPP
