//
// Created by wpsimon09 on 18/08/2025.
//

#include "DenoisePass.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Renderer/Renderers/RenderPass/RenderPass.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include <memory>
#include <vulkan/vulkan_enums.hpp>

namespace Renderer {
BilateralFilterPass::BilateralFilterPass(const VulkanCore::VDevice&       device,
                                         ApplicationCore::EffectsLibrary& effectsLibrary,
                                         VulkanCore::VImage2&             inputImage,
                                         int                              width,
                                         int                              height)
    : RenderPass(device, width, height)
    , m_inputImage(inputImage)
{

    assert(width >= inputImage.GetImageInfo().width && height >= inputImage.GetImageInfo().height
           && "You can not downscale the denoise image only upsale");

    //==============================================================================================
    // create and build effect, it has to be recreated so that it can be reused through the renderer
    m_bilateralFileter = std::make_unique<VulkanUtils::VComputeEffect>(device, "Bilateral filter", "Shaders/Compiled/Bilaterial-Filter.spv",
                                                                       effectsLibrary.GetDescriptorLayoutCache(),
                                                                       EShaderBindingGroup::ComputePostProecess);
    m_bilateralFileter->BuildEffect();

    //============================================================
    // create compute attachment
    Renderer::RenderTarget2CreatInfo denoisedResultCI;
    denoisedResultCI.width               = width;
    denoisedResultCI.heigh               = height;
    denoisedResultCI.format              = m_inputImage.GetImageInfo().format;
    denoisedResultCI.initialLayout       = vk::ImageLayout::eShaderReadOnlyOptimal;
    denoisedResultCI.isDepth             = false;
    denoisedResultCI.multiSampled        = false;
    denoisedResultCI.computeShaderOutput = true;

    denoisedResultCI.imageDebugName = "Denoise pass result | Reflections";
    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, denoisedResultCI));
}

void BilateralFilterPass::Init(int                                 currentFrameIndex,
                               VulkanUtils::VUniformBufferManager& uniformBufferManager,
                               VulkanUtils::RenderContext*         renderContext)
{
    m_bilateralFileter->SetNumWrites(1, 4, 0);

    //m_bilateralFileter->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);

    m_bilateralFileter->WriteImage(currentFrameIndex, 0, 0, m_inputImage.GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    m_bilateralFileter->WriteImage(currentFrameIndex, 0, 1,
                                   m_renderTargets[EBilateralFilterAttachments::Result]->GetPrimaryImage().GetDescriptorImageInfo());

    m_bilateralFileter->WriteImage(currentFrameIndex, 0, 2,
                                   renderContext->normalMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    m_bilateralFileter->ApplyWrites(currentFrameIndex);
}

void BilateralFilterPass::Update(int                                   currentFrame,
                                 VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                                 VulkanUtils::RenderContext*           renderContext,
                                 VulkanStructs::PostProcessingContext* postProcessingContext)
{
    m_bilateralFilterParameters = uniformBufferManager.GetApplicationState()->GetBilateralFilaterParameters();

    if(m_inputImage.GetImageInfo().width != m_width || m_inputImage.GetImageInfo().height != m_height)
    {
        m_bilateralFilterParameters.upscale      = true;
        m_bilateralFilterParameters.targetWdidth = m_renderTargets[EBilateralFilterAttachments::Result]->GetWidth();
        m_bilateralFilterParameters.targetHeight = m_renderTargets[EBilateralFilterAttachments::Result]->GetHeight();
    }

    m_bilateralFilterParameters.width  = m_inputImage.GetImageInfo().width;
    m_bilateralFilterParameters.height = m_inputImage.GetImageInfo().height;
}

void BilateralFilterPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    assert(cmdBuffer.GetIsRecording() && " Command buffer is not in recording state");


    VulkanUtils::VBarrierPosition barrier = {vk::PipelineStageFlagBits2::eColorAttachmentOutput | vk::PipelineStageFlagBits2::eComputeShader,
                                             vk::AccessFlagBits2::eColorAttachmentWrite | vk::AccessFlagBits2::eShaderWrite,
                                             vk::PipelineStageFlagBits2::eComputeShader,
                                             vk::AccessFlagBits2::eShaderSampledRead | vk::AccessFlagBits2::eShaderWrite};
    m_renderTargets[EBilateralFilterAttachments::Result]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eGeneral,
                                                                                vk::ImageLayout::eShaderReadOnlyOptimal, barrier);

    m_bilateralFileter->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_bilateralFileter->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_bilateralFileter->GetPipelineLayout();
    pcInfo.size       = sizeof(BilaterialFilterParameters);
    pcInfo.offset     = 0;
    pcInfo.pValues    = &m_bilateralFilterParameters;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_bilateralFileter->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);

    cmdBuffer.GetCommandBuffer().dispatch(VulkanUtils::celiDiv(m_bilateralFilterParameters.targetWdidth, 16),
                                          VulkanUtils::celiDiv(m_bilateralFilterParameters.targetHeight, 16), 1);

    VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite,
                                                vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                                                vk::AccessFlagBits2::eShaderRead};

    m_renderTargets[EBilateralFilterAttachments::Result]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                                                vk::ImageLayout::eGeneral, barrierPos);
}

void BilateralFilterPass::Destroy()
{
    RenderPass::Destroy();
    m_bilateralFileter->Destroy();
}

//=========================================================
// Very simple upscaler
//=========================================================

UpscalePass::UpscalePass(const VulkanCore::VDevice&       device,
                         ApplicationCore::EffectsLibrary& effectsLibrary,
                         VulkanCore::VImage2&             inputImage,
                         int                              targetWidth,
                         int                              targetHeight)
    : RenderPass(device, targetWidth, targetHeight)
    , m_inputImage(inputImage)
{
    Renderer::RenderTarget2CreatInfo upscaleResultCI;
    upscaleResultCI.width               = targetWidth;
    upscaleResultCI.heigh               = targetHeight;
    upscaleResultCI.format              = inputImage.GetImageInfo().format;
    upscaleResultCI.initialLayout       = vk::ImageLayout::eShaderReadOnlyOptimal;
    upscaleResultCI.isDepth             = false;
    upscaleResultCI.multiSampled        = false;
    upscaleResultCI.computeShaderOutput = true;

    upscaleResultCI.imageDebugName = "Upscaler result pass";
    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, upscaleResultCI));

    m_upsacleEffect = std::make_unique<VulkanUtils::VComputeEffect>(device, "Bilateral filter", "Shaders/Compiled/Upscale.spv",
                                                                    effectsLibrary.GetDescriptorLayoutCache(),
                                                                    EShaderBindingGroup::ComputePostProecess);
    m_upsacleEffect->BuildEffect();
}

void UpscalePass::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    m_upsacleEffect->SetNumWrites(0, 2, 0);
    m_upsacleEffect->WriteImage(currentFrameIndex, 0, 0, m_inputImage.GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    m_upsacleEffect->WriteImage(currentFrameIndex, 0, 1, GetPrimaryAttachemntDescriptorInfo(0));
    m_upsacleEffect->ApplyWrites(currentFrameIndex);
}

void UpscalePass::Update(int                                   currentFrame,
                         VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                         VulkanUtils::RenderContext*           renderContext,
                         VulkanStructs::PostProcessingContext* postProcessingContext)
{
}

void UpscalePass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
                                              VulkanUtils::VImage_SampledRead_To_General);

    m_upsacleEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_upsacleEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);
    cmdBuffer.GetCommandBuffer().dispatch(VulkanUtils::celiDiv(m_width, 16), VulkanUtils::celiDiv(m_height, 16), 1);

    m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
                                              VulkanUtils::VImage_SampledRead_To_General.Switch());
}

void UpscalePass::Destroy()
{
    RenderPass::Destroy();
}

}  // namespace Renderer