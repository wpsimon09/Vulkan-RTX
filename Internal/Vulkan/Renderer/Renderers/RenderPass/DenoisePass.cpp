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

namespace Renderer {
BilateralFilterPass::BilateralFilterPass(const VulkanCore::VDevice&       device,
                                         ApplicationCore::EffectsLibrary& effectsLibrary,
                                         VulkanCore::VImage2&             inputImage,
                                         int                              width,
                                         int                              height)
    : RenderPass(device, width, height)
    , m_inputImage(inputImage)
{

    //===========================================================
    // create and build effect
    m_bilateralFileter = effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::BilateralFilter);
    //============================================================
    // create compute attachment
    Renderer::RenderTarget2CreatInfo denoisedResultCI;
    denoisedResultCI.width               = width;
    denoisedResultCI.heigh               = height;
    denoisedResultCI.format              = vk::Format::eR16G16B16A16Sfloat;
    denoisedResultCI.initialLayout       = vk::ImageLayout::eShaderReadOnlyOptimal;
    denoisedResultCI.isDepth             = false;
    denoisedResultCI.multiSampled        = false;
    denoisedResultCI.computeShaderOutput = true;

    denoisedResultCI.imageDebugName = "Denoise pass result";
    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, denoisedResultCI));
}

void BilateralFilterPass::Init(int                                 currentFrameIndex,
                               VulkanUtils::VUniformBufferManager& uniformBufferManager,
                               VulkanUtils::RenderContext*         renderContext)
{
    m_bilateralFileter->SetNumWrites(1, 4, 0);

    //m_bilateralFileter->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);

    m_bilateralFileter->WriteImage(currentFrameIndex, 0, 1, m_inputImage.GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    m_bilateralFileter->WriteImage(currentFrameIndex, 0, 2,
                                   m_renderTargets[EBilateralFilterAttachments::Result]->GetPrimaryImage().GetDescriptorImageInfo());

    m_bilateralFileter->WriteImage(currentFrameIndex, 0, 3,
                                   renderContext->normalMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    m_bilateralFileter->ApplyWrites(currentFrameIndex);
}

void BilateralFilterPass::Update(int                                   currentFrame,
                                 VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                                 VulkanUtils::RenderContext*           renderContext,
                                 VulkanStructs::PostProcessingContext* postProcessingContext)
{
    m_bilateralFilterParameters = uniformBufferManager.GetApplicationState()->GetBilateralFilaterParameters();

    m_bilateralFilterParameters.width =
        m_renderTargets[EBilateralFilterAttachments::Result]->GetPrimaryImage().GetImageInfo().width;
    m_bilateralFilterParameters.height =
        m_renderTargets[EBilateralFilterAttachments::Result]->GetPrimaryImage().GetImageInfo().height;
}

void BilateralFilterPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    assert(cmdBuffer.GetIsRecording() && " Command buffer is not in recording state");

    VulkanUtils::VBarrierPosition barrier = {vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite,
                                             vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite};
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

    cmdBuffer.GetCommandBuffer().dispatch(m_bilateralFilterParameters.width / 16, m_bilateralFilterParameters.height / 16, 1);

    VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite,
                                                vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderRead};

    m_renderTargets[EBilateralFilterAttachments::Result]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                                                vk::ImageLayout::eGeneral, barrierPos);
}

void BilateralFilterPass::Destroy()
{
    RenderPass::Destroy();
    m_bilateralFileter->Destroy();
}

}  // namespace Renderer