//
// Created by wpsimon09 on 16/08/2025.
//

#ifndef RTSHADOWMAPPASS_HPP
#define RTSHADOWMAPPASS_HPP
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "RenderPass.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"

namespace VulkanCore {
class VCommandBuffer;
}
namespace VulkanUtils {
struct RenderContext;
class VRasterEffect;
}  // namespace VulkanUtils
namespace Renderer {

enum EVisibilityBufferAttachments
{
    ShadowMap = 0,
};

class VisibilityBufferPass : public Renderer::RenderPass
{
  public:
    VisibilityBufferPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height);

    void Init(int frameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;

    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

    void Destroy() override;

  private:
    std::shared_ptr<VulkanUtils::VComputeEffect> m_rayTracedShadowEffect;
    AoOcclusionParameters                        m_aoOcclusionParameters;
};


class AoOcclusionPass : public Renderer::RenderPass
{
  public:
    AoOcclusionPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height);
    void Init(int frameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;

    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

    void Destroy() override;

  private:
    std::shared_ptr<VulkanUtils::VComputeEffect> m_aoEffect;
    std::unique_ptr<VulkanCore::VImage2>         m_previousFrame;
    AoOcclusionParameters                        m_aoOcclusionParameters;
    bool                                         m_accumulate;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_RTSHADOWMAPPASS_HPP
