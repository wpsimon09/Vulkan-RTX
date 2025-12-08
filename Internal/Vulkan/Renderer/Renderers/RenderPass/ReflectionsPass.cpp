//
// Created by wpsimon09 on 08/12/2025.
//

#include "ReflectionsPass.hpp"

namespace Renderer {

RayTracedReflectionsPass::RayTracedReflectionsPass(const VulkanCore::VDevice&       device,
                                                   ApplicationCore::EffectsLibrary& effectsLibrary,
                                                   int                              width,
                                                   int                              height) : RenderPass(device, width, height)
{

}
void RayTracedReflectionsPass::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
}
void RayTracedReflectionsPass::Update(int                                   currentFrame,
                             VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                             VulkanUtils::RenderContext*           renderContext,
                             VulkanStructs::PostProcessingContext* postProcessingContext)
{
}
void RayTracedReflectionsPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
}
}  // namespace Renderer