//
// Created by wpsimon09 on 08/12/2025.
//

#ifndef VULKAN_RTX_REFLECTIONSPASS_HPP
#define VULKAN_RTX_REFLECTIONSPASS_HPP
#include "RenderPass.hpp"

namespace VulkanUtils {
class VComputeEffect;
}
namespace Renderer {

class RayTracedReflectionsPass : public Renderer::RenderPass
{
  public:
    RayTracedReflectionsPass(const VulkanCore::VDevice& device,ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height);

    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;
    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;
    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;
private:
    std::shared_ptr<VulkanUtils::VComputeEffect> m_rayTracedReflectionEffect;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_REFLECTIONSPASS_HPP
