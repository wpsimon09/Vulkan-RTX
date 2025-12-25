//
// Created by wpsimon09 on 16/08/2025.
//

#include "VisibilityBufferPass.hpp"
#include "RenderPass.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"

namespace Renderer {
VisibilityBufferPass::VisibilityBufferPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height)
    : Renderer::RenderPass(device, width, height)
    , m_aoOcclusionParameters{}
{
    //=================================================
    // create the effect
    m_rayTracedShadowEffect = effectLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::RTShadowPass);

    //===================================================
    // create render target
    Renderer::RenderTarget2CreatInfo shadowMapCI{
        width, height, false, false, vk::Format::eR16Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ResolveModeFlagBits::eNone, true, "visibility buffer attachment"};

    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, shadowMapCI));
}
void VisibilityBufferPass::Init(int frameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{

    m_rayTracedShadowEffect->SetNumWrites(3, 5, 1);

    m_rayTracedShadowEffect->WriteBuffer(frameIndex, 0, 0, uniformBufferManager.GetLightBufferDescriptorInfo()[frameIndex]);

    m_rayTracedShadowEffect->WriteAccelerationStrucutre(frameIndex, 0, 1, renderContext->tlas);

    m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 2,
                                        renderContext->positionMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerDepth));

    m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 3,
                                        MathUtils::LookUpTables.BlueNoise1024->GetHandle()->GetDescriptorImageInfo(
                                            VulkanCore::VSamplers::Sampler2D));

    m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 4,
                                        renderContext->normalMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerDepth));

    m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 5,
                                        GetPrimaryAttachemntDescriptorInfo(EVisibilityBufferAttachments::ShadowMap));

    m_rayTracedShadowEffect->ApplyWrites(frameIndex);
}

void VisibilityBufferPass::Update(int                                   currentFrame,
                                  VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                                  VulkanUtils::RenderContext*           renderContext,
                                  VulkanStructs::PostProcessingContext* postProcessingContext)
{
    m_rayTracedShadowEffect->SetNumWrites(0, 0, 1);
    m_rayTracedShadowEffect->WriteAccelerationStrucutre(currentFrame, 0, 1, renderContext->tlas);
    m_rayTracedShadowEffect->ApplyWrites(currentFrame);

    m_aoOcclusionParameters = uniformBufferManager.GetApplicationState()->GetAoOcclusionParameters();
}

void VisibilityBufferPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{

    VulkanUtils::PlacePipelineBarrier(cmdBuffer, vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                      vk::PipelineStageFlagBits::eFragmentShader);

    //=========================================================================
    // Transition shadow map from shader read only optimal to general
    m_renderTargets[EVisibilityBufferAttachments::ShadowMap]->TransitionAttachments(
        cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, VulkanUtils::VImage_SampledRead_To_General);


    auto& cmdB = cmdBuffer.GetCommandBuffer();

    m_rayTracedShadowEffect->BindPipeline(cmdB);
    m_rayTracedShadowEffect->BindDescriptorSet(cmdB, currentFrame, 0);

    //dispatch

    cmdB.dispatch(m_width / 16, (m_height / 16) + 1, 1);

    m_renderTargets[EVisibilityBufferAttachments::ShadowMap]->TransitionAttachments(
        cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
        VulkanUtils::VImage_SampledRead_To_General.Switch());
}

void VisibilityBufferPass::Destroy()
{
    RenderPass::Destroy();
    m_rayTracedShadowEffect->Destroy();
}


//************************************************************************ */
//********************************* RT- Ambient occlusion **************** */
//************************************************************************ */
AoOcclusionPass::AoOcclusionPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height)
    : Renderer::RenderPass(device, width, height)
    , m_aoOcclusionParameters{}
{
    //=================================================
    // create the effect
    m_aoEffect = effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::RT_AoOcclusionPass);

    //===================================================
    // create render target
    Renderer::RenderTarget2CreatInfo aoPassCi{width,
                                              height,
                                              false,
                                              false,
                                              vk::Format::eR16Sfloat,
                                              vk::ImageLayout::eShaderReadOnlyOptimal,
                                              vk::ResolveModeFlagBits::eNone,
                                              true,
                                              "AO Pass attachment",
                                              vk::ImageUsageFlagBits::eTransferSrc};

    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(device, aoPassCi));

    VulkanCore::VImage2CreateInfo previousImageCI{width,
                                                  height,
                                                  4,
                                                  1,
                                                  "generated",
                                                  EImageSource::Generated,
                                                  1,
                                                  1,
                                                  vk::Format::eR16Sfloat,
                                                  vk::ImageAspectFlagBits::eColor,
                                                  vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
                                                  vk::SampleCountFlagBits::e1,
                                                  vk::ImageLayout::eShaderReadOnlyOptimal,
                                                  "Previously sampled AO",
                                                  "Previously sampled AO",
                                                  false};

    m_previousFrame = std::make_unique<VulkanCore::VImage2>(m_device, previousImageCI);

    VulkanUtils::PlaceImageMemoryBarrier2(*m_previousFrame, m_device.GetTransferOpsManager().GetCommandBuffer(),
                                          vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
                                          VulkanUtils::VImage_Undefined_ToShaderRead);
}

void AoOcclusionPass::Init(int frameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    m_aoEffect->SetNumWrites(0, 5, 1);

    m_aoEffect->WriteImage(frameIndex, 0, 0, renderContext->normalMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    m_aoEffect->WriteImage(frameIndex, 0, 1, renderContext->positionMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    m_aoEffect->WriteImage(frameIndex, 0, 2, GetPrimaryAttachemntDescriptorInfo(0));
    m_aoEffect->WriteAccelerationStrucutre(frameIndex, 0, 3, renderContext->tlas);
    m_aoEffect->WriteImage(frameIndex, 0, 4, m_previousFrame->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    m_aoEffect->WriteImage(frameIndex, 0, 5, renderContext->motionVector->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));


    m_aoEffect->ApplyWrites(frameIndex);
}

void AoOcclusionPass::Update(int                                   currentFrame,
                             VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                             VulkanUtils::RenderContext*           renderContext,
                             VulkanStructs::PostProcessingContext* postProcessingContext)
{
    m_aoEffect->SetNumWrites(0, 0, 1);
    m_aoEffect->WriteAccelerationStrucutre(currentFrame, 0, 3, renderContext->tlas);
    m_aoEffect->ApplyWrites(currentFrame);

    m_aoOcclusionParameters              = uniformBufferManager.GetApplicationState()->GetAoOcclusionParameters();
    m_aoOcclusionParameters.currentFrame = m_device.CurrentFrame;
}

void AoOcclusionPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
                                              VulkanUtils::VImage_SampledRead_To_General);

    m_aoEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_aoEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_aoEffect->GetPipelineLayout();
    pcInfo.offset     = 0;
    pcInfo.size       = sizeof(m_aoOcclusionParameters);
    pcInfo.pValues    = &m_aoOcclusionParameters;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_aoEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);
    cmdBuffer.GetCommandBuffer().dispatch(m_width / 16, m_height / 16, 1);


    VulkanUtils::VBarrierPosition barrierPos;
    if((bool)m_aoOcclusionParameters.accumulate)
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
        VulkanUtils::CopyImageWithBarriers(m_width, m_height, cmdBuffer, m_renderTargets[0]->GetPrimaryImage(), *m_previousFrame);

        barrierPos = {vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead,
                      vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                      vk::AccessFlagBits2::eShaderSampledRead};

        // storage image now will be read so read only layout
        m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                  vk::ImageLayout::eTransferSrcOptimal, barrierPos);
    }
    else
    {
        m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal,
                                                  VulkanUtils::VImage_SampledRead_To_General.Switch());
    }
}

void AoOcclusionPass::Destroy()
{
    RenderPass::Destroy();
    m_previousFrame->Destroy();
}


}  // namespace Renderer