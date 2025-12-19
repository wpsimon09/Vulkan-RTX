//
// Created by wpsimon09 on 08/12/2025.
//

#include "ReflectionsPass.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include <exception>
#include <memory>
#include <vulkan/vulkan_enums.hpp>
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
namespace Renderer {

RayTracedReflectionsPass::RayTracedReflectionsPass(const VulkanCore::VDevice&       device,
                                                   ApplicationCore::EffectsLibrary& effectsLibrary,
                                                   int                              width,
                                                   int                              height)
    : RenderPass(device, width, height)
{
    m_rayTracedReflectionEffect =
        effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::RT_Reflections);

    Renderer::RenderTarget2CreatInfo rtReflectionTargetCi{width,
                                                          height,
                                                          false,
                                                          false,
                                                          vk::Format::eR16G16B16A16Sfloat,
                                                          vk::ImageLayout::eShaderReadOnlyOptimal,
                                                          vk::ResolveModeFlagBits::eNone,
                                                          true,
                                                          "Ray traced reflection output"};

    // frame 1
    m_renderTargets.push_back(std::make_unique<RenderTarget2>(device, rtReflectionTargetCi));

    // frame 2
    m_renderTargets.push_back(std::make_unique<RenderTarget2>(device, rtReflectionTargetCi));
}

void RayTracedReflectionsPass::Init(int                                 currentFrameIndex,
                                    VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                    VulkanUtils::RenderContext*         renderContext)
{
    m_rayTracedReflectionEffect->SetNumWrites(1, 8, 1);

    m_rayTracedReflectionEffect->WriteImage(currentFrameIndex, 0, 0, GetPrimaryAttachemntDescriptorInfo(0));
    m_rayTracedReflectionEffect->WriteImage(currentFrameIndex, 0, 1,
                                            renderContext->positionMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    m_rayTracedReflectionEffect->WriteImage(currentFrameIndex, 0, 2,
                                            renderContext->depthBuffer->GetResolvedImage().GetDescriptorImageInfo(
                                                VulkanCore::VSamplers::Sampler2D));
    m_rayTracedReflectionEffect->WriteImage(currentFrameIndex, 0, 3,
                                            renderContext->normalMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    m_rayTracedReflectionEffect->WriteImage(currentFrameIndex, 0, 4,
                                            renderContext->armMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    m_rayTracedReflectionEffect->WriteBuffer(currentFrameIndex, 0, 5,
                                             uniformBufferManager.GetGlobalBufferDescriptorInfo2(currentFrameIndex));

    m_rayTracedReflectionEffect->WriteImage(currentFrameIndex, 0, 6,
                                            renderContext->motionVector->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    m_rayTracedReflectionEffect->ApplyWrites(currentFrameIndex);
}

void RayTracedReflectionsPass::Update(int                                   currentFrame,
                                      VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                                      VulkanUtils::RenderContext*           renderContext,
                                      VulkanStructs::PostProcessingContext* postProcessingContext)
{

    m_rayTracedReflectionEffect->SetNumWrites(3, uniformBufferManager.GetAll2DTextureDescriptorImageInfo().size() + 4, 1);
    if(currentFrame == 1)
    {
        m_rayTracedReflectionEffect->WriteImage(currentFrame, 0, 7,
                                                GetPrimaryAttachemntDescriptorInfo(0, VulkanCore::VSamplers::Sampler2D));
    }
    else
    {
        m_rayTracedReflectionEffect->WriteImage(currentFrame, 0, 7,
                                                GetPrimaryAttachemntDescriptorInfo(1, VulkanCore::VSamplers::Sampler2D));
    }

    m_rayTracedReflectionEffect->WriteBuffer(currentFrame, 1, 0, renderContext->rtxObjectBufer);
    m_rayTracedReflectionEffect->WriteBuffer(currentFrame, 1, 1, uniformBufferManager.GetMaterialDescriptionBuffer(currentFrame));
    ;
    m_rayTracedReflectionEffect->WriteImageArray(currentFrame, 1, 2, uniformBufferManager.GetAll2DTextureDescriptorImageInfo());
    m_rayTracedReflectionEffect->WriteAccelerationStrucutre(currentFrame, 1, 3, renderContext->tlas);
    m_rayTracedReflectionEffect->ApplyWrites(currentFrame);
}

void RayTracedReflectionsPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    m_renderTargets[currentFrame]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                         VulkanUtils::VImage_SampledRead_To_General);

    m_rayTracedReflectionEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_rayTracedReflectionEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

    cmdBuffer.GetCommandBuffer().dispatch(m_width / 16, m_height / 16, 1);

    m_renderTargets[currentFrame]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
                                                         VulkanUtils::VImage_SampledRead_To_General.Switch());
}
}  // namespace Renderer