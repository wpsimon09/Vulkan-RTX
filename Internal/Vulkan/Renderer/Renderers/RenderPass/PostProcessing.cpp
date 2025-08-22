//
// Created by wpsimon09 on 21/08/2025.
//

#include "PostProcessing.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace Renderer {
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
void FogPass::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    auto& e = m_fogPassEffect;
    e->SetNumWrites(4, 7, 2);

    e->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);

    e->WriteImage(currentFrameIndex, 0, 1, renderContext->visibilityBuffer->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    e->WriteImage(currentFrameIndex, 0, 2, renderContext->positionMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerDepth));
    e->WriteImage(currentFrameIndex, 0, 3,
                  MathUtils::LookUpTables.BlueNoise1024->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    e->WriteImage(currentFrameIndex, 0, 4, renderContext->lightPassOutput->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    e->WriteBuffer(currentFrameIndex, 0, 5, uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrameIndex]);

    e->WriteAccelerationStrucutre(currentFrameIndex, 0, 6, renderContext->tlas);


    e->ApplyWrites(currentFrameIndex);
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
    m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                              vk::ImageLayout::eShaderReadOnlyOptimal);

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

    m_renderTargets[0]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                              vk::ImageLayout::eColorAttachmentOptimal);
}

//=============================================================================
// ************************* TONE MAPPING *************************************

ToneMapping::ToneMapping(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height)
    : RenderPass(device, width, height)
    , m_luminanceHistogramAverageParameters{}
    , m_luminanceHistogramParameters{}
    , m_toneMappingParameters{}
{
    //========================================
    // retrieve effects
    m_luminanceHistogramEffect =
        effectLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::LuminanceHistrogram);
    m_luminanceAverageEffect = effectLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::AverageLuminance);
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
        width, height, false, false, vk::Format::eR32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ResolveModeFlagBits::eNone, true};
    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, toneMapOutputCI));


    VulkanStructs::VImageData<float> dummyImageData = {};
    dummyImageData.channels                         = 4;
    dummyImageData.height                           = 1;
    dummyImageData.widht                            = 1;
    float* dummyPixels                              = new float[]{0.0};
    dummyImageData.pixels                           = dummyPixels;

    m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage().FillWithImageData<float>(
        dummyImageData, device.GetTransferOpsManager().GetCommandBuffer());

    m_device.GetTransferOpsManager().DestroyBuffer(
        m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage().GetImageStagingvBuffer(), true);
}
void ToneMapping::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    //===========================
    // Tone mapping descriptor
    auto& e = m_toneMappingEffect;
    e->SetNumWrites(1, 2, 0);

    e->WriteImage(currentFrameIndex, 0, 0,
                  renderContext->depthBuffer->GetResolvedImage().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    e->ApplyWrites(currentFrameIndex);

    //================================
    // Luminance histogram descriptors
    m_luminanceHistogramEffect->SetNumWrites(1, 1);

    m_luminanceHistogramEffect->WriteBuffer(currentFrameIndex, 0, 1, uniformBufferManager.GetLuminanceHistogram(currentFrameIndex));

    m_luminanceHistogramEffect->ApplyWrites(currentFrameIndex);

    //================================
    // Average luminance
    m_luminanceAverageEffect->SetNumWrites(1, 1);

    m_luminanceAverageEffect->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetLuminanceHistogram(currentFrameIndex));
    m_luminanceAverageEffect->WriteImage(currentFrameIndex, 0, 1,
               m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage().GetDescriptorImageInfo());
    m_luminanceAverageEffect->ApplyWrites(currentFrameIndex);
}

void ToneMapping::Update(int                                   currentFrame,
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

    if(postProcessingContext->luminanceAverageParameters != nullptr)
    {
        m_luminanceAverageEffect->SetNumWrites(0, 1, 0);

        m_luminanceAverageEffect->ApplyWrites(currentFrame);
    }


    m_toneMappingEffect->SetNumWrites(0, 2, 0);

    m_toneMappingEffect->WriteImage(currentFrame, 1, 0,
                                    postProcessingContext->sceneRender->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    m_toneMappingEffect->WriteImage(
        currentFrame, 1, 1, m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage().GetDescriptorImageInfo());
    m_toneMappingEffect->ApplyWrites(currentFrame);

    m_toneMappingParameters = *postProcessingContext->toneMappingParameters;

    m_luminanceHistogramParameters                   = *postProcessingContext->luminanceHistrogramParameters;
    m_luminanceHistogramAverageParameters.pixelCount = postProcessingContext->sceneRender->GetPixelCount();
    //    m_luminanceHistogramAverageParameters.logLuminanceRange = postProcessingContext->luminanceHistrogramParameters->logRange;
    //    m_luminanceHistogramAverageParameters.minLogLuminance = postProcessingContext->luminanceHistrogramParameters->minLogLuminance;
    m_luminanceHistogramAverageParameters.timeDelta = postProcessingContext->deltaTime;


    m_luminanceHistogramParameters.width  = m_width;
    m_luminanceHistogramParameters.height = m_height;
}
void ToneMapping::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    //==============================================================
    // AUTO EXPOSURE FIRST
    float w = m_width;
    float h = m_height;

    //================================================
    // Luminance histogram
    //================================================

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout = m_luminanceAverageEffect->GetPipelineLayout();
    pcInfo.size = sizeof(LuminanceHistogramParameters) - 2 * sizeof(float);  // one parameter is not taken into the account
    pcInfo.offset     = 0;
    pcInfo.pValues    = &m_luminanceHistogramParameters;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_luminanceHistogramEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_luminanceHistogramEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);
    m_luminanceHistogramEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);


    cmdBuffer.GetCommandBuffer().dispatch(w / 16, h / 16, 1);

    //=================================================
    // Average luminance
    //=================================================
    VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage(),
                                                    vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                    cmdBuffer.GetCommandBuffer());
    pcInfo.layout     = m_luminanceAverageEffect->GetPipelineLayout();
    pcInfo.size       = sizeof(LuminanceHistogramAverageParameters);  // one parameter is not taken into the account
    pcInfo.offset     = 0;
    pcInfo.pValues    = &m_luminanceHistogramParameters;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_luminanceAverageEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_luminanceAverageEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);
    m_luminanceAverageEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);

    cmdBuffer.GetCommandBuffer().dispatch(1, 1, 1);

    VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets[EToneMappingAttachments::LuminanceAverage]->GetPrimaryImage(),
                                                    vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
                                                    cmdBuffer.GetCommandBuffer());

    //=======================================================
    // Tone mapping
    //=======================================================
    m_renderTargets[EToneMappingAttachments::LDR]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                                                         vk::ImageLayout::eShaderReadOnlyOptimal);

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
    pcInfo.layout     = m_toneMappingEffect->GetPipelineLayout();
    pcInfo.size       = sizeof(ToneMappingParameters);
    pcInfo.offset     = 0;
    pcInfo.pValues    = &m_toneMappingParameters;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_toneMappingEffect->CmdPushConstant(cmdB, pcTonaMapping);


    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();

    m_renderTargets[EToneMappingAttachments::LDR]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                                         vk::ImageLayout::eColorAttachmentOptimal);
}


}  // namespace Renderer