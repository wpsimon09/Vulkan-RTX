//
// Created by wpsimon09 on 21/08/2025.
//

#include "PostProcessing.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/RenderPass.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include <exception>
#include <vector>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Renderer {

//=============================================================================
// ************************* FOG PASS *************************************
//=============================================================================
FogPass::FogPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height)
    : RenderPass(device, width, height)
    , m_parameters{}
{
    m_fogPassEffect = effectLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::FogVolume);

    //===========================
    // Generate attachment
    //==================
    // Fog pass output
    Renderer::RenderTarget2CreatInfo fogPassOutputCI{
        width,
        height,
        false,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eNone,
    };

    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, fogPassOutputCI));
}
void FogPass::Init(int currentFrame, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    auto& e = m_fogPassEffect;
    e->SetNumWrites(4, 7, 2);

    e->WriteBuffer(currentFrame, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrame]);

    e->WriteImage(currentFrame, 0, 1, renderContext->visibilityBuffer->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    e->WriteImage(currentFrame, 0, 2, renderContext->positionMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerDepth));

    e->WriteImage(currentFrame, 0, 3,
                  MathUtils::LookUpTables.BlueNoise1024->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    e->WriteImage(currentFrame, 0, 4, renderContext->lightPassOutput->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    e->WriteBuffer(currentFrame, 0, 5, uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrame]);

    e->WriteAccelerationStrucutre(currentFrame, 0, 6, renderContext->tlas);


    e->ApplyWrites(currentFrame);
}

void FogPass::Update(int                                   currentFrame,
                     VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                     VulkanUtils::RenderContext*           renderContext,
                     VulkanStructs::PostProcessingContext* postProcessingContext)
{
    auto& e = m_fogPassEffect;
    e->SetNumWrites(4, 7, 2);
    e->WriteAccelerationStrucutre(currentFrame, 0, 6, renderContext->tlas);
    e->ApplyWrites(currentFrame);


    if(uniformBufferManager.GetApplicationState()->GetFogVolumeParameters())
    {
        m_parameters = *uniformBufferManager.GetApplicationState()->GetFogVolumeParameters();
    }
}

void FogPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    // TODO: fog volume parameters are now push constant instead of UBO
    // this might not be the best thing to do but for now it should suffice

    m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                                              VulkanUtils::VRenderTarget_ToAttachment_FromSample);

    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {m_renderTargets[0]->GenerateAttachmentInfo(
        vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)};

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

    m_fogPassEffect->BindPipeline(cmdB);
    m_fogPassEffect->BindDescriptorSet(cmdB, currentFrame, 0);


    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_fogPassEffect->GetPipelineLayout();
    pcInfo.size       = sizeof(FogVolumeParameters);
    pcInfo.offset     = 0;
    pcInfo.pValues    = &m_parameters;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_fogPassEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);


    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();

    m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
                                              VulkanUtils::VRenderTarget_Color_ToSample_InShader_BarrierPosition);
}

void FogPass::Destroy()
{
    RenderPass::Destroy();
    m_fogPassEffect->Destroy();
}

//=============================================================================
// ************************* TONE MAPPING *************************************
//=============================================================================

ToneMappingPass::ToneMappingPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height)
    : RenderPass(device, width, height)
    , m_averageLuminanceParameters{}
    , m_luminanceHistogramParameters{}
    , m_toneMappingParameters{}
{
    //========================================
    // retrieve effects
    m_luminanceHistogramEffect =
        effectLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::LuminanceHistrogram);
    m_averageLuminanceEffect = effectLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::AverageLuminance);
    m_toneMappingEffect = effectLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::ToneMappingPass);

    //=========================================
    // create attachments

    // LDR - tone mapping output = 0
    Renderer::RenderTarget2CreatInfo toneMapOutputCI{
        width,
        height,
        false,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eNone,
    };
    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, toneMapOutputCI));

    // Average luminance = 1

    RenderTarget2CreatInfo avgLuminanceOutputCI{
        1, 1, false, false, vk::Format::eR32Sfloat, vk::ImageLayout::eUndefined, vk::ResolveModeFlagBits::eNone, true};
    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, avgLuminanceOutputCI));


    VulkanStructs::VImageData<float> dummyImageData = {};
    dummyImageData.channels                         = 4;
    dummyImageData.height                           = 1;
    dummyImageData.widht                            = 1;
    float* dummyPixels                              = new float[]{0.2};
    dummyImageData.pixels                           = dummyPixels;

    // this will transition the render target to shader read only optimal
    m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage().FillWithImageData<float>(
        dummyImageData, device.GetTransferOpsManager().GetCommandBuffer());

    m_device.GetTransferOpsManager().DestroyBuffer(
        m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage().GetImageStagingvBuffer(), true);

    VulkanUtils::VBarrierPosition barrierPos = {
        vk::PipelineStageFlagBits2::eCopy,
        vk::AccessFlagBits2::eTransferWrite,
        vk::PipelineStageFlagBits2::eComputeShader,
        vk::AccessFlagBits2::eShaderWrite,
    };
    m_renderTargets[EToneMappingAttachments::LuminanceAverage]->TransitionAttachments(
        device.GetTransferOpsManager().GetCommandBuffer(), vk::ImageLayout::eGeneral,
        vk::ImageLayout::eShaderReadOnlyOptimal, barrierPos);

    m_luminanceHistogramBuffer.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
}
void ToneMappingPass::Init(int currentFrame, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    //===========================
    // Tone mapping descriptor
    m_toneMappingEffect->SetNumWrites(1, 2, 0);

    m_toneMappingEffect->WriteImage(
        currentFrame, 0, 0, renderContext->depthBuffer->GetResolvedImage().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    m_toneMappingEffect->ApplyWrites(currentFrame);

    //================================
    // Luminance histogram descriptors
    m_luminanceHistogramEffect->SetNumWrites(1, 1);

    m_luminanceHistogramEffect->WriteBuffer(currentFrame, 0, 1, uniformBufferManager.GetLuminanceHistogram(currentFrame));

    m_luminanceHistogramEffect->ApplyWrites(currentFrame);

    m_luminanceHistogramBuffer[currentFrame] = uniformBufferManager.GetLuminanceHistogram(currentFrame).buffer;

    //================================
    // Average luminance
    m_averageLuminanceEffect->SetNumWrites(1, 1);

    m_averageLuminanceEffect->WriteBuffer(currentFrame, 0, 0, uniformBufferManager.GetLuminanceHistogram(currentFrame));
    m_averageLuminanceEffect->WriteImage(
        currentFrame, 0, 1, m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage().GetDescriptorImageInfo());

    m_averageLuminanceEffect->ApplyWrites(currentFrame);

    //=====================================
}

void ToneMappingPass::Update(int                                   currentFrame,
                             VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                             VulkanUtils::RenderContext*           renderContext,
                             VulkanStructs::PostProcessingContext* postProcessingContext)
{
    if(postProcessingContext->sceneRender != nullptr)
    {
        m_luminanceHistogramEffect->SetNumWrites(0, 1);
        m_luminanceHistogramEffect->WriteImage(
            currentFrame, 0, 0, postProcessingContext->sceneRender->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
        m_luminanceHistogramEffect->ApplyWrites(currentFrame);
    }

    m_toneMappingEffect->SetNumWrites(0, 2, 0);
    if(postProcessingContext->sceneRender != nullptr)
    {
        m_toneMappingEffect->WriteImage(currentFrame, 1, 0,
                                        postProcessingContext->sceneRender->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    }
    m_toneMappingEffect->WriteImage(
        currentFrame, 1, 1, m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage().GetDescriptorImageInfo());
    m_toneMappingEffect->ApplyWrites(currentFrame);

    m_averageLuminanceEffect->SetNumWrites(1, 1);

    m_averageLuminanceEffect->WriteBuffer(currentFrame, 0, 0, uniformBufferManager.GetLuminanceHistogram(currentFrame));
    m_averageLuminanceEffect->WriteImage(
        currentFrame, 0, 1, m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage().GetDescriptorImageInfo());

    m_averageLuminanceEffect->ApplyWrites(currentFrame);

    //===============================
    // Fill in the push constant data

    m_toneMappingParameters        = *postProcessingContext->toneMappingParameters;
    m_luminanceHistogramParameters = *postProcessingContext->luminanceHistrogramParameters;

    m_averageLuminanceParameters.pixelCount =
        postProcessingContext->sceneRender == nullptr ? 0 : postProcessingContext->sceneRender->GetPixelCount();
    m_averageLuminanceParameters.timeDelta         = postProcessingContext->deltaTime;
    m_averageLuminanceParameters.logLuminanceRange = postProcessingContext->luminanceHistrogramParameters->logRange;
    m_averageLuminanceParameters.minLogLuminance = postProcessingContext->luminanceHistrogramParameters->minLogLuminance;

    m_luminanceHistogramParameters.width  = m_width;
    m_luminanceHistogramParameters.height = m_height;
}

void ToneMappingPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    //==============================================================
    // AUTO EXPOSURE FIRST
    float w = m_width;
    float h = m_height;

    //================================================
    // Luminance histogram
    //================================================
    vk::PushConstantsInfo pcLuminanceHistogram;
    pcLuminanceHistogram.layout = m_averageLuminanceEffect->GetPipelineLayout();
    pcLuminanceHistogram.size = sizeof(LuminanceHistogramParameters) - 2 * sizeof(float);  // two parameters are not taken into the account
    pcLuminanceHistogram.offset     = 0;
    pcLuminanceHistogram.pValues    = &m_luminanceHistogramParameters;
    pcLuminanceHistogram.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_luminanceHistogramEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_luminanceHistogramEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);
    m_luminanceHistogramEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcLuminanceHistogram);

    cmdBuffer.GetCommandBuffer().dispatch(w / 16, h / 16, 1);

    //==================================
    // sync the average luminance buffer
    VulkanUtils::VBarrierPosition bufferBarrierPos = {vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite,
                                                      vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderRead};
    VulkanUtils::PlaceBufferMemoryBarrier2(cmdBuffer.GetCommandBuffer(), m_luminanceHistogramBuffer[currentFrame], bufferBarrierPos);

    //=================================================
    // Average luminance
    //=================================================
    VulkanUtils::VBarrierPosition imageBarrierPos = {vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderRead,
                                                     vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite};

    m_renderTargets[EToneMappingAttachments::LuminanceAverage]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eGeneral,
                                                                                      vk::ImageLayout::eGeneral, imageBarrierPos);

    vk::PushConstantsInfo pcLuminanceAverage;
    pcLuminanceAverage.layout = m_averageLuminanceEffect->GetPipelineLayout();
    pcLuminanceAverage.size = sizeof(LuminanceHistogramAverageParameters);  // one parameter is not taken into the account
    pcLuminanceAverage.offset     = 0;
    pcLuminanceAverage.pValues    = &m_averageLuminanceParameters;
    pcLuminanceAverage.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_averageLuminanceEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_averageLuminanceEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);
    m_averageLuminanceEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcLuminanceAverage);

    cmdBuffer.GetCommandBuffer().dispatch(1, 1, 1);

    imageBarrierPos = {vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite,
                       vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                       vk::AccessFlagBits2::eShaderSampledRead};

    m_renderTargets[EToneMappingAttachments::LuminanceAverage]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eGeneral,
                                                                                      vk::ImageLayout::eGeneral, imageBarrierPos);


    //=======================================================
    // Tone mapping
    //=======================================================
    imageBarrierPos = {vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eFragmentShader,
                       vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                       vk::AccessFlagBits2::eColorAttachmentWrite};

    m_renderTargets[EToneMappingAttachments::LDR]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                                                         vk::ImageLayout::eShaderReadOnlyOptimal, imageBarrierPos);


    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {m_renderTargets[EToneMappingAttachments::LDR]->GenerateAttachmentInfo(
        vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)};

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = renderingOutputs.size();
    renderingInfo.pColorAttachments    = renderingOutputs.data();
    renderingInfo.pDepthAttachment     = nullptr;

    auto& cmdB = cmdBuffer.GetCommandBuffer();

    cmdB.beginRendering(&renderingInfo);

    // TODO: call ConfigureViewPort() here
    vk::Viewport viewport{
        0, 0, (float)renderingInfo.renderArea.extent.width, (float)renderingInfo.renderArea.extent.height, 0.0f, 1.0f};
    cmdB.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{{0, 0},
                        {(uint32_t)renderingInfo.renderArea.extent.width, (uint32_t)renderingInfo.renderArea.extent.height}};
    cmdB.setScissor(0, 1, &scissors);
    cmdB.setStencilTestEnable(false);


    m_toneMappingEffect->BindPipeline(cmdB);
    m_toneMappingEffect->BindDescriptorSet(cmdB, currentFrame, 0);

    vk::PushConstantsInfo pcTonaMapping;
    pcTonaMapping.layout     = m_toneMappingEffect->GetPipelineLayout();
    pcTonaMapping.size       = sizeof(ToneMappingParameters);
    pcTonaMapping.offset     = 0;
    pcTonaMapping.pValues    = &m_toneMappingParameters;
    pcTonaMapping.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_toneMappingEffect->CmdPushConstant(cmdB, pcTonaMapping);


    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();


    m_renderTargets[EToneMappingAttachments::LDR]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                                         vk::ImageLayout::eColorAttachmentOptimal,
                                                                         imageBarrierPos.Switch());
}

void ToneMappingPass::Destroy()
{
    RenderPass::Destroy();
    m_averageLuminanceEffect->Destroy();
    m_luminanceHistogramEffect->Destroy();
    m_toneMappingEffect->Destroy();
}

//=======================================================================================================
//***************************************** LENS FLARE **************************************************
//=======================================================================================================

LensFlarePass::LensFlarePass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height)
    : RenderPass(device, width, height)
    , m_lensFlareParameters{}
{
    //============================
    // Retrieve the effect
    m_lensFlareEffect = effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::LensFlare);

    //==================================
    // Generate lens flare render target
    RenderTarget2CreatInfo lensFlareOutputCI{
        width,
        height,
        false,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eNone,
    };

    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(device, lensFlareOutputCI));
}

void LensFlarePass::Init(int currentFrame, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    m_lensFlareEffect->SetNumWrites(3, 2, 0);

    m_lensFlareEffect->WriteBuffer(currentFrame, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrame]);

    m_lensFlareEffect->WriteImage(currentFrame, 0, 1,
                                  MathUtils::LookUpTables.BlueNoise1024->GetHandle()->GetDescriptorImageInfo(
                                      VulkanCore::VSamplers::Sampler2D));

    m_lensFlareEffect->WriteBuffer(currentFrame, 0, 2, uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrame]);

    m_lensFlareEffect->ApplyWrites(currentFrame);
}

void LensFlarePass::Update(int                                   currentFrame,
                           VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                           VulkanUtils::RenderContext*           renderContext,
                           VulkanStructs::PostProcessingContext* postProcessingContext)
{
    //===========================
    // update push constant values
    if(postProcessingContext->lensFlareParameters)
    {
        m_lensFlareParameters = *postProcessingContext->lensFlareParameters;
    }
    if(postProcessingContext->sceneRender != nullptr)
    {
        m_lensFlareEffect->SetNumWrites(0, 1, 0);
        m_lensFlareEffect->WriteImage(currentFrame, 1, 0,
                                      postProcessingContext->sceneRender->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
        m_lensFlareEffect->ApplyWrites(currentFrame);
    }
}

void LensFlarePass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderRead,
                                                vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                vk::AccessFlagBits2::eColorAttachmentWrite};
    m_renderTargets[ELensFlareAttachments::LensFlareMain]->TransitionAttachments(
        cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, barrierPos);

    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {m_renderTargets[ELensFlareAttachments::LensFlareMain]->GenerateAttachmentInfo(
        vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)};

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = renderingOutputs.size();
    renderingInfo.pColorAttachments    = renderingOutputs.data();
    renderingInfo.pDepthAttachment     = nullptr;

    auto& cmdB = cmdBuffer.GetCommandBuffer();

    cmdB.beginRendering(&renderingInfo);

    vk::Viewport viewport{
        0, 0, (float)renderingInfo.renderArea.extent.width, (float)renderingInfo.renderArea.extent.height, 0.0f, 1.0f};
    cmdB.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{{0, 0},
                        {(uint32_t)renderingInfo.renderArea.extent.width, (uint32_t)renderingInfo.renderArea.extent.height}};
    cmdB.setScissor(0, 1, &scissors);
    cmdB.setStencilTestEnable(false);

    m_lensFlareEffect->BindPipeline(cmdB);
    m_lensFlareEffect->BindDescriptorSet(cmdB, currentFrame, 0);

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_lensFlareEffect->GetPipelineLayout();
    pcInfo.size       = sizeof(LensFlareParameters);
    pcInfo.offset     = 0;
    pcInfo.pValues    = &m_lensFlareParameters;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_lensFlareEffect->CmdPushConstant(cmdB, pcInfo);

    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();

    m_renderTargets[ELensFlareAttachments::LensFlareMain]->TransitionAttachments(
        cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal, barrierPos.Switch());
}

void LensFlarePass::Destroy()
{
    RenderPass::Destroy();
    m_lensFlareEffect->Destroy();
}

//=======================================================================================================
//***************************************** BLOOM PASS **************************************************
//=======================================================================================================

BloomPass::BloomPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height)
    : RenderPass(device, width, height)
    , m_downSampleParams{}
    , m_upSampleParams{}
{
    RenderTarget2CreatInfo bloomOutputCi{width,
                                         height,
                                         false,
                                         false,
                                         vk::Format::eR16G16B16A16Sfloat,
                                         vk::ImageLayout::eGeneral,
                                         vk::ResolveModeFlagBits::eNone,
                                         true};
    m_renderTargets.resize(EBloomAttachments::Count);

    for(int i = 0; i < EBloomAttachments::Count; i++)
    {
        bloomOutputCi.width *= 0.5;
        bloomOutputCi.heigh *= 0.5;
        m_renderTargets[i] = std::make_unique<Renderer::RenderTarget2>(device, bloomOutputCi);
    }

    bloomOutputCi.width                              = width;
    bloomOutputCi.heigh                              = height;
    m_renderTargets[EBloomAttachments::BloomFullRes] = std::make_unique<Renderer::RenderTarget2>(device, bloomOutputCi);

    // TODO: create attachment that will combine the results from output and bloom
    /*
        E' = E                  // E, D, C, B, A are downsampled
        D' = D + blur(E', b4)   // b_x is the filter radius 
        C' = C + blur(D', b3)
        B' = B + blur(C', b2)
        A' = A + blur(B', b1)

       [FullRes](1) [A](0.5 res) [B](0.25 res) [C](0.125 res) [D].... - this is how down sampling looks like 
    */


    m_downSampleEffect = effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::BloomDownSample);
    m_upSampleEffect = effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::BloomUpSample);

    m_downSampleWriteImages.resize(EBloomAttachments::Count);
    m_downSampleReadImages.resize(EBloomAttachments::Count);  // + 1 for full res HDR colour attachment


    m_upSampleReadImage.resize(EBloomAttachments::Count);
    m_upSampleWriteImages.resize(EBloomAttachments::Count);  // + 1 for full res Bloom output
}

void BloomPass::Init(int currentFrame, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{

    /*
        down sample reads - rendered texture, A, B , C, D (5)
        down sample writes - A`, B`, C`, D`, E` (5)

        up sample reads - A, B, C, D, E (5)
        up sample write - bloom output, A, B, C, D, E (5)
    */

    m_downSampleEffect->SetNumWrites(0, 2 * (EBloomAttachments::Count + 1), 0);  //+1 for the original image
    m_upSampleEffect->SetNumWrites(0, 2 * (EBloomAttachments::Count + 1), 0);

    // this will get replaced udring update
    m_downSampleWriteImages[0] =
        m_renderTargets[EBloomAttachments::BloomFullRes]->GetPrimaryImage().GetDescriptorImageInfo()[EBloomAttachments::BloomFullRes];

    for(int i = 1; i < EBloomAttachments::Count - 1; i++)
    {
        m_downSampleReadImages[i] =
            m_renderTargets[i - 1]->GetPrimaryImage().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
    }

    for(int i = 0; i < EBloomAttachments::Count - 1; i++)
    {
        m_downSampleWriteImages[i] = m_renderTargets[i - 1]->GetPrimaryImage().GetDescriptorImageInfo();
    }

    for(int i = 0; i < EBloomAttachments::Count - 1)

        // i can update up sample fist image now, since it will be the bloom pass output attachment, where full res image is combined with up sampled image
        m_upSampleWriteImages[0] = m_renderTargets[EBloomAttachments::BloomFullRes]->GetPrimaryImage().GetDescriptorImageInfo();

    // to eliminite segv i will write bloom attachment to the sampled reads for the down sample effect
    m_downSampleReadImages[0] = m_renderTargets[EBloomAttachments::BloomFullRes]->GetPrimaryImage().GetDescriptorImageInfo(
        VulkanCore::VSamplers::Sampler2D)[EBloomAttachments::BloomFullRes];


    for(int i = 1; i < EBloomAttachments::Count; i++)
    {
        // sampled images
        // Goes from A to E

        m_upSampleReadImage[i] = m_renderTargets[i - 1]->GetPrimaryImage().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

        // rw textures
        // Goes from E` to A`
        m_downSampleWriteImages[i] = m_renderTargets[i - 1]->GetPrimaryImage().GetDescriptorImageInfo();
        m_upSampleWriteImages[i]   = m_renderTargets[i - 1]->GetPrimaryImage().GetDescriptorImageInfo();
    }

    m_downSampleEffect->WriteImageArray(currentFrame, 0, 0, m_downSampleReadImages);
    m_downSampleEffect->WriteImageArray(currentFrame, 0, 1, m_downSampleWriteImages);

    m_upSampleEffect->WriteImageArray(currentFrame, 0, 0, m_upSampleReadImage);
    m_upSampleEffect->WriteImageArray(currentFrame, 0, 1, m_upSampleWriteImages);


    m_upSampleEffect->ApplyWrites(currentFrame);
    m_downSampleEffect->ApplyWrites(currentFrame);
}

void BloomPass::Update(int                                   currentFrame,
                       VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                       VulkanUtils::RenderContext*           renderContext,
                       VulkanStructs::PostProcessingContext* postProcessingContext)
{
    // todo: final composition of the images will require bloom strength that might be put here
    //TODO create effect that will merge both up sampled image (bloom output) and final scene render, in opengl version they use GL_BLEND_ADDITIVE which i cant do with compute shaders
    //m_writeImages[0]   = postProcessingContext->sceneRender->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

    m_downSampleEffect->SetNumWrites(0, EBloomAttachments::Count, 0);

    // down sample has as a first image input as a scene render whic is used as a first thing to down sample (HDR colour)
    m_downSampleReadImages[0] = postProcessingContext->sceneRender->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
    m_downSampleEffect->WriteImageArray(currentFrame, 0, 0, m_downSampleReadImages);

    //up sampling outputs and inputs does not vary based on the previous render, therfore it is not included here

    m_downSampleParams.src_xy_dst_xy.x = postProcessingContext->sceneRender->GetImageInfo().width;
    m_downSampleParams.src_xy_dst_xy.y = postProcessingContext->sceneRender->GetImageInfo().height;
    m_downSampleParams.srcImage        = 0;  // start with the scene render
}


void BloomPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    //========================================
    // Down sample
    // - loop over each mip
    // - write correct resoruces (dispatch shader on smaller level, and sample higher )
    // - ( the down sample takes one mip larger as an input and outpus one mip smaller with applied bluer )

    // bind resources
    // I start from 1 since
    // : HDR image [0], A [1], B[2]
    for(int i = 1; i < EBloomAttachments::Count; i++)  // + 1 to include full res image
    {

        // source
        m_downSampleParams.src_xy_dst_xy.x = m_renderTargets[i - 1]->GetWidth();
        m_downSampleParams.src_xy_dst_xy.y = m_renderTargets[i - 1]->GetHeight();
        m_downSampleParams.srcImage        = i - 1;

        // destination
        m_downSampleParams.src_xy_dst_xy.z = m_renderTargets[i]->GetWidth();
        m_downSampleParams.src_xy_dst_xy.w = m_renderTargets[i]->GetHeight();
        m_downSampleParams.dstImage        = i;

        m_downSampleEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
        m_downSampleEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

        // set up push-constatnts
        vk::PushConstantsInfo pcInfo;
        pcInfo.layout     = m_downSampleEffect->GetPipelineLayout();
        pcInfo.size       = sizeof(m_downSampleParams);
        pcInfo.offset     = 0;
        pcInfo.pValues    = &m_downSampleParams;
        pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

        m_downSampleEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);

        cmdBuffer.GetCommandBuffer().dispatch(m_downSampleParams.src_xy_dst_xy.x / 8, m_downSampleParams.src_xy_dst_xy.y / 8, 1);

        VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite,
                                                    vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderRead};

        VulkanUtils::PlaceImageMemoryBarrier2(m_renderTargets[i]->GetPrimaryImage(), cmdBuffer,
                                              vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, barrierPos);
    }

    //============================================
    // up sample

    // TODO: extract only bright parts of the scene
}

void BloomPass::Destroy()
{
    m_downSampleEffect->Destroy();
    m_upSampleEffect->Destroy();
    RenderPass::Destroy();
}


}  // namespace Renderer