//
// Created by wpsimon09 on 17/08/2025.
//

#ifndef VULKAN_RTX_GBUFFERPASS_HPP
#define VULKAN_RTX_GBUFFERPASS_HPP
#include "RenderPass.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"

namespace Renderer {

enum EGBufferAttachments
{
    Position = 0,
    Normal,
    Albedo,
    // put all attachments above this
    Size
};

class GBufferPass : public Renderer::RenderPass
{
  public:
    GBufferPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height);

    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;

    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

    void Destroy() override;

    RenderTarget2& GetDepthAttachment();

  private:
    int                                         m_numGBufferAttachments = EGBufferAttachments::Size;
    std::shared_ptr<VulkanUtils::VRasterEffect> m_gBufferEffect;
    std::unique_ptr<Renderer::RenderTarget2>    m_depthBuffer;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_GBUFFERPASS_HPP
