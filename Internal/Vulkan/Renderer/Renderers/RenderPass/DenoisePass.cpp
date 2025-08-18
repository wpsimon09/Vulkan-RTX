//
// Created by wpsimon09 on 18/08/2025.
//

#include "DenoisePass.hpp"

#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"

namespace Renderer {
BilateralFilterPass::BilateralFilterPass(const VulkanCore::VDevice&          device,
                                         VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                                         const VulkanCore::VImage2&          inputImage,
                                         int                                 width,
                                         int                                 height)
    : RenderPass(device, width, height)
    , m_inputImage(inputImage)
{

  //===========================================================
  // create and build effect
  m_bilateralFileter = std::make_unique<VulkanUtils::VComputeEffect>(m_device, "BilaterialPass", "Shaders/Compiled/Bilaterial-Filter.spv",
                                                        descLayoutCache, EShaderBindingGroup::ComputePostProecess);
  m_bilateralFileter->BuildEffect();

  //============================================================
  // create compute attachment
  Renderer::RenderTarget2CreatInfo denoisedResultCI;
  denoisedResultCI.width = width;
  denoisedResultCI.heigh = height;
  denoisedResultCI.format = vk::Format::eR16G16B16A16Sfloat;
  denoisedResultCI.initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  denoisedResultCI.isDepth = false;
  denoisedResultCI.multiSampled = false;
  denoisedResultCI.computeShaderOutput = true;

  m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, denoisedResultCI));




}

void BilateralFilterPass::Init(int                                 currentFrameIndex,
                               VulkanUtils::VUniformBufferManager& uniformBufferManager,
                               VulkanUtils::RenderContext*         renderContext)
{
}

void BilateralFilterPass::Update(int                                   currentFrame,
                                 VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                                 VulkanUtils::RenderContext*           renderContext,
                                 VulkanStructs::PostProcessingContext* postProcessingContext)
{
}

void BilateralFilterPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
}

}  // namespace Renderer