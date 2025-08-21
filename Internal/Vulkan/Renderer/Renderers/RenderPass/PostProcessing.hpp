//
// Created by wpsimon09 on 21/08/2025.
//

#ifndef VULKAN_RTX_POSTPROCESSING_HPP
#define VULKAN_RTX_POSTPROCESSING_HPP
#include "RenderPass.hpp"

namespace Renderer {

enum FogPassAttachments
{
    FogMain = 0
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

}  // namespace Renderer

#endif  //VULKAN_RTX_POSTPROCESSING_HPP
