//
// Created by wpsimon09 on 28/09/2025.
//

#ifndef VULKAN_RTX_ATMOSPHEREPASS_HPP
#define VULKAN_RTX_ATMOSPHEREPASS_HPP
#include "RenderPass.hpp"

namespace VulkanStructs {
struct PostProcessingContext;
}
namespace VulkanUtils {
struct RenderContext;
class VUniformBufferManager;
}  // namespace VulkanUtils
namespace ApplicationCore {
class EffectsLibrary;
}
namespace VulkanCore {
class VCommandBuffer;
class VImage2;
}  // namespace VulkanCore
namespace Renderer {

class AtmospherePass : public Renderer::RenderPass
{
    AtmospherePass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height);
    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;

    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

    void Destroy() override;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_ATMOSPHEREPASS_HPP
