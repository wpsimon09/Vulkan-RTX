//
// Created by wpsimon09 on 28/09/2025.
//

#include "AtmospherePass.hpp"

#include "RenderPass.hpp"

namespace Renderer {
AtmospherePass::AtmospherePass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height):RenderPass(device, width, height)
{

}

void AtmospherePass::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{

}

void AtmospherePass::Update(int                                   currentFrame,
                            VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                            VulkanUtils::RenderContext*           renderContext,
                            VulkanStructs::PostProcessingContext* postProcessingContext)
{

}

void AtmospherePass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{

}

void AtmospherePass::Destroy() {

}

}  // namespace Renderer