//
// Created by wpsimon09 on 08/12/2025.
//

#include "ReflectionsPass.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Renderer/Renderers/RenderPass/DenoisePass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/RenderPass.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include <cstdint>
#include <exception>
#include <memory>
#include <vulkan/vulkan_enums.hpp>
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "vulkan/vulkan.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/DenoisePass.hpp"

namespace Renderer {

RayTracedReflectionsPass::RayTracedReflectionsPass(const VulkanCore::VDevice&       device,
                                                   ApplicationCore::EffectsLibrary& effectsLibrary,
                                                   int                              width,
                                                   int                              height,
                                                   int                              targetWidth,
                                                   int                              targetHeight)
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
                                                          "Ray traced reflection output - 0",
                                                          vk::ImageUsageFlagBits::eTransferSrc};

    // frame 1
    m_renderTargets.emplace_back(std::make_unique<RenderTarget2>(device, rtReflectionTargetCi));

    VulkanCore::VImage2CreateInfo previousImageCI{width,
                                                  height,
                                                  4,
                                                  1,
                                                  "generated",
                                                  EImageSource::Generated,
                                                  1,
                                                  1,
                                                  vk::Format::eR16G16B16A16Sfloat,
                                                  vk::ImageAspectFlagBits::eColor,
                                                  vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
                                                  vk::SampleCountFlagBits::e1,
                                                  vk::ImageLayout::eShaderReadOnlyOptimal,
                                                  "Previously sampled RT refletction",
                                                  "Previously sampled RT refletction",
                                                  false};

    m_previousImage = std::make_unique<VulkanCore::VImage2>(m_device, previousImageCI);

    VulkanUtils::PlaceImageMemoryBarrier2(*m_previousImage, m_device.GetTransferOpsManager().GetCommandBuffer(),
                                          vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
                                          VulkanUtils::VImage_Undefined_ToShaderRead);

    m_denoiser = std::make_unique<Renderer::BilateralFilterPass>(m_device, effectsLibrary,
                                                                 m_renderTargets[0]->GetPrimaryImage(), targetWidth, targetHeight);
}

void RayTracedReflectionsPass::Init(int                                 currentFrameIndex,
                                    VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                    VulkanUtils::RenderContext*         renderContext)
{


    m_rayTracedReflectionEffect->SetNumWrites(1, 9, 1);

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

    m_rayTracedReflectionEffect->WriteImage(currentFrameIndex, 0, 7,
                                            m_previousImage->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));


    m_rayTracedReflectionEffect->WriteImage(currentFrameIndex, 0, 8,
                                            renderContext->albedoMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    m_rayTracedReflectionEffect->ApplyWrites(currentFrameIndex);

    m_denoiser->Init(currentFrameIndex, uniformBufferManager, renderContext);
}

void RayTracedReflectionsPass::Update(int                                   currentFrame,
                                      VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                                      VulkanUtils::RenderContext*           renderContext,
                                      VulkanStructs::PostProcessingContext* postProcessingContext)
{

    m_rayTracedReflectionEffect->SetNumWrites(3, uniformBufferManager.GetAll2DTextureDescriptorImageInfo().size() + 6, 1);


    m_rayTracedReflectionEffect->WriteBuffer(currentFrame, 1, 0, renderContext->rtxObjectBufer);
    m_rayTracedReflectionEffect->WriteBuffer(currentFrame, 1, 1, uniformBufferManager.GetMaterialDescriptionBuffer(currentFrame));

    m_rayTracedReflectionEffect->WriteImageArray(currentFrame, 1, 2, uniformBufferManager.GetAll2DTextureDescriptorImageInfo());


    m_rayTracedReflectionEffect->WriteAccelerationStrucutre(currentFrame, 1, 3, renderContext->tlas);

    m_rayTracedReflectionEffect->ApplyWrites(currentFrame);

    m_reflectionsParameters = uniformBufferManager.GetApplicationState()->GetReflectionsParameters();

    m_denoiser->Update(currentFrame, uniformBufferManager, renderContext, postProcessingContext);
}

void RayTracedReflectionsPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
                                              VulkanUtils::VImage_SampledRead_To_General);

    m_rayTracedReflectionEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_rayTracedReflectionEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_rayTracedReflectionEffect->GetPipelineLayout();
    pcInfo.offset     = 0;
    pcInfo.size       = sizeof(m_reflectionsParameters);
    pcInfo.pValues    = &m_reflectionsParameters;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_rayTracedReflectionEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);

    cmdBuffer.GetCommandBuffer().dispatch(m_width / 16, m_height / 16, 1);

    VulkanUtils::VBarrierPosition barrierPos;

    if((bool)m_reflectionsParameters.accumulate)
    {

        barrierPos = VulkanUtils::VBarrierPosition{vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite,
                                                   vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferRead};
        // storage image now will be read so read only layout
        m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eTransferSrcOptimal,
                                                  vk::ImageLayout::eGeneral, barrierPos);

        //======================================
        // Copy the result to the previous image
        // - make previous transfer dst
        // - copy the values
        // - make shader read only again
        VulkanUtils::CopyImageWithBarriers(m_width, m_height, cmdBuffer, m_renderTargets[0]->GetPrimaryImage(), *m_previousImage);

        barrierPos = {vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead,
                      vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                      vk::AccessFlagBits2::eShaderSampledRead};
        // storage image now will be read so read only layout
        m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                  vk::ImageLayout::eTransferSrcOptimal, barrierPos);
    }
    else
    {
        // storage image now will be read so read only layout
        m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
                                                  VulkanUtils::VImage_SampledRead_To_General.Switch());
    }

    m_denoiser->Render(currentFrame, cmdBuffer, renderContext);
}

RenderTarget2* RayTracedReflectionsPass::GetAccumulatedResult() const
{
    // return whatever was rendered last and is in hte shader read only position
    return &m_denoiser->GetRenderTarget(0);
}

void RayTracedReflectionsPass::Destroy()
{
    RenderPass::Destroy();
    m_previousImage->Destroy();
}

}  // namespace Renderer