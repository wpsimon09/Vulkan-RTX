//
// Created by wpsimon09 on 28/09/2025.
//

#include "AtmospherePass.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Renderer/Renderers/RenderPass/RenderPass.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include <exception>
#include <memory>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Renderer {
AtmospherePass::AtmospherePass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height)
    : RenderPass(device, width, height)
{
    m_transmitanceLutEffect =
        effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::AtmosphereTransmitanceLUT);

    m_multipleScatteringLutEffect =
        effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::MultipleScatteringLUT);

    m_skyViewLutEffect = effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::SkyViewLUT);


    m_atmospherePassEffect = effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::AtmospherePass);

    Renderer::RenderTarget2CreatInfo transmitanceLutCi{256,
                                                       64,
                                                       false,
                                                       false,
                                                       vk::Format::eR16G16B16A16Sfloat,
                                                       vk::ImageLayout::eGeneral,
                                                       vk::ResolveModeFlagBits::eNone,
                                                       true};
    transmitanceLutCi.imageDebugName = "Transmitacne LUT";
    m_renderTargets.push_back(std::make_unique<RenderTarget2>(device, transmitanceLutCi));

    transmitanceLutCi.heigh          = 32;
    transmitanceLutCi.width          = 32;
    transmitanceLutCi.imageDebugName = "Multiscattering LUT";
    m_renderTargets.push_back(std::make_unique<RenderTarget2>(device, transmitanceLutCi));

    transmitanceLutCi.width          = 192;
    transmitanceLutCi.heigh          = 128;
    transmitanceLutCi.imageDebugName = "Sky view-lut";
    m_renderTargets.push_back(std::make_unique<RenderTarget2>(device, transmitanceLutCi));
}

void AtmospherePass::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    renderContext->transmitanceLut = &m_renderTargets[EAtmosphereAttachments::TransmitanceLUT]->GetPrimaryImage();
    renderContext->skyViewLut      = &m_renderTargets[EAtmosphereAttachments::SkyViewLut]->GetPrimaryImage();


    m_transmitanceLutEffect->SetNumWrites(0, 1, 0);
    m_transmitanceLutEffect->WriteImage(currentFrameIndex, 0, 0,
                                        GetPrimaryAttachemntDescriptorInfo(EAtmosphereAttachments::TransmitanceLUT));
    m_transmitanceLutEffect->ApplyWrites(currentFrameIndex);

    //=============================

    m_multipleScatteringLutEffect->SetNumWrites(0, 2, 0);
    // precomputed transmitance
    m_multipleScatteringLutEffect->WriteImage(currentFrameIndex, 0, 0,
                                              GetPrimaryAttachemntDescriptorInfo(EAtmosphereAttachments::TransmitanceLUT,
                                                                                 VulkanCore::VSamplers::Sampler2D));
    //output
    m_multipleScatteringLutEffect->WriteImage(currentFrameIndex, 0, 1,
                                              GetPrimaryAttachemntDescriptorInfo(EAtmosphereAttachments::MultipleScatteringLut));

    m_multipleScatteringLutEffect->ApplyWrites(currentFrameIndex);

    //=============================

    m_skyViewLutEffect->SetNumWrites(2, 3, 0);
    m_skyViewLutEffect->WriteImage(currentFrameIndex, 0, 0,
                                   GetPrimaryAttachemntDescriptorInfo(EAtmosphereAttachments::TransmitanceLUT,
                                                                      VulkanCore::VSamplers::Sampler2D));
    m_skyViewLutEffect->WriteImage(currentFrameIndex, 0, 1,
                                   GetPrimaryAttachemntDescriptorInfo(EAtmosphereAttachments::MultipleScatteringLut,
                                                                      VulkanCore::VSamplers::Sampler2D));

    m_skyViewLutEffect->WriteImage(currentFrameIndex, 0, 2, GetPrimaryAttachemntDescriptorInfo(EAtmosphereAttachments::SkyViewLut));
    m_skyViewLutEffect->WriteBuffer(currentFrameIndex, 0, 3, uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrameIndex]);
    m_skyViewLutEffect->WriteBuffer(currentFrameIndex, 0, 4, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);

    m_skyViewLutEffect->ApplyWrites(currentFrameIndex);

    //=============================

    m_atmospherePassEffect->SetNumWrites(2, 5, 0);
    m_atmospherePassEffect->WriteBuffer(currentFrameIndex, 0, 0,
                                        uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);
    m_atmospherePassEffect->WriteBuffer(currentFrameIndex, 0, 1,
                                        uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrameIndex]);
    m_atmospherePassEffect->WriteImage(currentFrameIndex, 0, 2,
                                       renderContext->depthBuffer->GetResolvedImage().GetDescriptorImageInfo(
                                           VulkanCore::VSamplers::Sampler2D));
    m_atmospherePassEffect->WriteImage(currentFrameIndex, 0, 3,
                                       GetPrimaryAttachemntDescriptorInfo(EAtmosphereAttachments::SkyViewLut,
                                                                          VulkanCore::VSamplers::Sampler2D));
    m_atmospherePassEffect->WriteImage(currentFrameIndex, 0, 4,
                                       GetPrimaryAttachemntDescriptorInfo(EAtmosphereAttachments::TransmitanceLUT,
                                                                          VulkanCore::VSamplers::Sampler2D));


    m_atmospherePassEffect->ApplyWrites(currentFrameIndex);
}

void AtmospherePass::Update(int                                   currentFrame,
                            VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                            VulkanUtils::RenderContext*           renderContext,
                            VulkanStructs::PostProcessingContext* postProcessingContext)
{
    if(uniformBufferManager.GetApplicationState()->GetAtmosphereParameters())
        m_atmosphereParams = *uniformBufferManager.GetApplicationState()->GetAtmosphereParameters();
}

void AtmospherePass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    //!NOTE: it will render to the final rendered image and not to its own

    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {renderContext->lightPassOutputRenderTarget->GenerateAttachmentInfoFromResolvedImage(
        vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore)};

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = renderingOutputs.size();
    renderingInfo.pColorAttachments    = renderingOutputs.data();
    renderingInfo.pDepthAttachment     = nullptr;

    auto& cmdB = cmdBuffer.GetCommandBuffer();

    cmdB.beginRendering(&renderingInfo);

    Renderer::ConfigureViewPort(cmdB, renderingInfo.renderArea.extent.width, renderingInfo.renderArea.extent.height);

    cmdB.setStencilTestEnable(false);

    m_atmospherePassEffect->BindPipeline(cmdB);
    m_atmospherePassEffect->BindDescriptorSet(cmdB, currentFrame, 0);

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_atmospherePassEffect->GetPipelineLayout();
    pcInfo.offset     = 0;
    pcInfo.pValues    = &m_atmosphereParams;
    pcInfo.size       = sizeof(m_atmosphereParams);
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_atmospherePassEffect->CmdPushConstant(cmdB, pcInfo);

    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();

    if(!renderContext->fogDrawCall.has_value())
    {
        VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                                                    vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                    vk::AccessFlagBits2::eColorAttachmentWrite};

        renderContext->lightPassOutputRenderTarget->TransitionAttachments(
            cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eAttachmentOptimal, barrierPos.Switch());
    }
}

void AtmospherePass::Precompute(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{

    m_transmitanceLutEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_transmitanceLutEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_transmitanceLutEffect->GetPipelineLayout();
    pcInfo.offset     = 0;
    pcInfo.pValues    = &m_atmosphereParams;
    pcInfo.size       = sizeof(m_atmosphereParams);
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_transmitanceLutEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);

    VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                                                vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eComputeShader,
                                                vk::AccessFlagBits2::eShaderWrite};

    VulkanUtils::PlaceImageMemoryBarrier2(m_renderTargets[EAtmosphereAttachments::TransmitanceLUT]->GetPrimaryImage(),
                                          cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, barrierPos);


    cmdBuffer.GetCommandBuffer().dispatch(m_renderTargets[EAtmosphereAttachments::TransmitanceLUT]->GetWidth() / 16,
                                          m_renderTargets[EAtmosphereAttachments::TransmitanceLUT]->GetHeight() / 16, 1);

    barrierPos = {vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eShaderStorageWrite,
                  vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                  vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderSampledRead};

    VulkanUtils::PlaceImageMemoryBarrier2(m_renderTargets[EAtmosphereAttachments::TransmitanceLUT]->GetPrimaryImage(),
                                          cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, barrierPos);

    //=====================================
    // Multiple scattering LUT
    m_multipleScatteringLutEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_multipleScatteringLutEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

    pcInfo.layout = m_multipleScatteringLutEffect->GetPipelineLayout();

    m_multipleScatteringLutEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);

    VulkanUtils::PlaceImageMemoryBarrier2(m_renderTargets[EAtmosphereAttachments::MultipleScatteringLut]->GetPrimaryImage(),
                                          cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, barrierPos);


    cmdBuffer.GetCommandBuffer().dispatch(m_renderTargets[EAtmosphereAttachments::MultipleScatteringLut]->GetWidth(),
                                          m_renderTargets[EAtmosphereAttachments::MultipleScatteringLut]->GetHeight(), 1);

    VulkanUtils::PlaceImageMemoryBarrier2(m_renderTargets[EAtmosphereAttachments::MultipleScatteringLut]->GetPrimaryImage(),
                                          cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, barrierPos);


    //========================================
    // Sky view LUT
    m_skyViewLutEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_skyViewLutEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

    pcInfo.layout = m_skyViewLutEffect->GetPipelineLayout();

    m_skyViewLutEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);

    barrierPos.srcPipelineStage |= vk::PipelineStageFlagBits2::eFragmentShader;
    barrierPos.srcData |= vk::AccessFlagBits2::eShaderSampledRead;
    barrierPos.dstData |= vk::AccessFlagBits2::eShaderWrite;
    VulkanUtils::PlaceImageMemoryBarrier2(m_renderTargets[EAtmosphereAttachments::SkyViewLut]->GetPrimaryImage(),
                                          cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, barrierPos);


    cmdBuffer.GetCommandBuffer().dispatch(m_renderTargets[EAtmosphereAttachments::SkyViewLut]->GetWidth() / 16,
                                          m_renderTargets[EAtmosphereAttachments::SkyViewLut]->GetHeight() / 16, 1);

    VulkanUtils::PlaceImageMemoryBarrier2(m_renderTargets[EAtmosphereAttachments::SkyViewLut]->GetPrimaryImage(),
                                          cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, barrierPos);
}

void AtmospherePass::Destroy()
{
    RenderPass::Destroy();
}

}  // namespace Renderer