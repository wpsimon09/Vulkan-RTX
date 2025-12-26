//
// Created by wpsimon09 on 08/12/2025.
//

#ifndef VULKAN_RTX_REFLECTIONSPASS_HPP
#define VULKAN_RTX_REFLECTIONSPASS_HPP
#include "Application/Structs/ParameterStructs.hpp"
#include "RenderPass.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"

namespace VulkanUtils {
class VComputeEffect;
}
namespace Renderer {
class RayTracedReflectionsPass : public Renderer::RenderPass
{
  public:
    RayTracedReflectionsPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height);

    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;
    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;
    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

    void Destroy() override;

    RenderTarget2* GetAccumulatedResult() const;

  private:
    std::shared_ptr<VulkanUtils::VComputeEffect> m_rayTracedReflectionEffect;
    std::unique_ptr<VulkanCore::VImage2>         m_previousImage;

    ReflectionsParameters m_reflectionsParameters;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_REFLECTIONSPASS_HPP
