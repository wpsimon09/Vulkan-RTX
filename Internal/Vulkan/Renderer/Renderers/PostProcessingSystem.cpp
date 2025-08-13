//
// Created by wpsimon09 on 13/06/25.
//

#include "PostProcessingSystem.h"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VIimageTransitionCommands.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include <memory>
#include <vulkan/vulkan_enums.hpp>

namespace Renderer {

PostProcessingSystem::PostProcessingSystem(const VulkanCore::VDevice&          device,
                                           ApplicationCore::EffectsLibrary&    effectsLibrary,
                                           VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                           int                                 width,
                                           int                                 height)
    : m_device(device)
    , m_uniformBufferManager(uniformBufferManager)
    , m_width(width)
    , m_height(height)
{
    Utils::Logger::LogInfo("Creating post processing system");

    //==================================
    // Get luminance histogram effect
    //==================================
    m_luminanceHistrogram = effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::LuminanceHistrogram);

    //==================================
    // Get average luminance effect
    //==================================
    m_averageLuminanceEffect = effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::AverageLuminance);

    //==================================
    // Get the tone mapping effect
    //==================================
    m_toneMappingEffect = effectsLibrary.GetEffect(ApplicationCore::EEffectType::ToneMappingPass);

    //====================================
    // Create tone mapping result image
    //===================================
    Renderer::RenderTarget2CreatInfo toneMapOutputCI{
        width,
        height,
        false,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eNone,
    };

    m_toneMapOutput = std::make_unique<Renderer::RenderTarget2>(m_device, toneMapOutputCI);


    //=========================
    // Get lens flare effect
    //=========================
    m_lensFlareEffect = effectsLibrary.GetEffect(ApplicationCore::EEffectType::LensFlare);

    //===============================
    // Create lens flare result image
    //===============================
    RenderTarget2CreatInfo lensFlareOutputCI{
        width,
        height,
        false,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eNone,
    };

    m_lensFlareOutput = std::make_unique<Renderer::RenderTarget2>(m_device, lensFlareOutputCI);

    //================================
    // Create avg luminance output
    //================================
    VulkanCore::VImage2CreateInfo avgLuminanceOutCi;
    avgLuminanceOutCi.width               = 1;
    avgLuminanceOutCi.height              = 1;
    avgLuminanceOutCi.format              = vk::Format::eR32Sfloat;
    avgLuminanceOutCi.isStorage           = true;
    avgLuminanceOutCi.layout              = vk::ImageLayout::eShaderReadOnlyOptimal;
    avgLuminanceOutCi.channels            = 1;
    avgLuminanceOutCi.imageAllocationName = "Average luminance";
    avgLuminanceOutCi.imageUsage |= vk::ImageUsageFlagBits::eStorage;

    m_averageLuminanceOutput = std::make_unique<VulkanCore::VImage2>(m_device, avgLuminanceOutCi);
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_averageLuminanceOutput, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                    vk::ImageLayout::eUndefined,
                                                    m_device.GetTransferOpsManager().GetCommandBuffer());

    Utils::Logger::LogInfo("Post processing system created");
}

void PostProcessingSystem::Render(int frameIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext)
{

    AutoExposure(frameIndex, commandBuffer, postProcessingContext);
    if(postProcessingContext.lensFlareEffect)
    {
        LensFlare(frameIndex, commandBuffer, postProcessingContext);
    }

    ToneMapping(frameIndex, commandBuffer, postProcessingContext);
}

void PostProcessingSystem::Update(int frameIndex, VulkanStructs::PostProcessingContext& postProcessingCotext)
{
    if(postProcessingCotext.sceneRender != nullptr)
    {
        m_luminanceHistrogram->SetNumWrites(0, 1);
        m_luminanceHistrogram->WriteImage(
            frameIndex, 0, 0, postProcessingCotext.sceneRender->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
        m_luminanceHistrogram->ApplyWrites(frameIndex);
    }

    if(postProcessingCotext.luminanceAverageParameters != nullptr)
    {
        m_averageLuminanceEffect->SetNumWrites(0, 1, 0);
        m_averageLuminanceEffect->WriteImage(frameIndex, 0, 1, m_averageLuminanceOutput->GetDescriptorImageInfo());
        m_averageLuminanceEffect->ApplyWrites(frameIndex);
    }

    if(postProcessingCotext.sceneRender != nullptr)
    {
        m_lensFlareEffect->SetNumWrites(0, 1, 0);
        m_lensFlareEffect->WriteImage(frameIndex, 1, 0,
                                      postProcessingCotext.sceneRender->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
        m_lensFlareEffect->ApplyWrites(frameIndex);
    }

    m_toneMappingEffect->SetNumWrites(0, 2, 0);

    if(postProcessingCotext.lensFlareEffect)
    {
        m_toneMappingEffect->WriteImage(
            frameIndex, 1, 0, m_lensFlareOutput->GetPrimaryImage().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    }
    else
    {
        m_toneMappingEffect->WriteImage(frameIndex, 1, 0,
                                        postProcessingCotext.sceneRender->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    }
    m_toneMappingEffect->WriteImage(frameIndex, 1, 1, m_averageLuminanceOutput->GetDescriptorImageInfo());

    m_toneMappingEffect->ApplyWrites(frameIndex);
}


VulkanCore::VImage2& PostProcessingSystem::GetRenderedResult(int frameIndex)
{
    return m_toneMapOutput->GetPrimaryImage();
}


void PostProcessingSystem::ToneMapping(int                                   currentIndex,
                                       VulkanCore::VCommandBuffer&           commandBuffer,
                                       VulkanStructs::PostProcessingContext& postProcessingContext)
{
    //=================================================================
    // Transition tone mapping output to the output attachment optimal
    m_toneMapOutput->TransitionAttachments(commandBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                           vk::ImageLayout::eShaderReadOnlyOptimal);

    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {m_toneMapOutput->GenerateAttachmentInfo(
        vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)};

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = renderingOutputs.size();
    renderingInfo.pColorAttachments    = renderingOutputs.data();
    renderingInfo.pDepthAttachment     = nullptr;

    auto& cmdB = commandBuffer.GetCommandBuffer();

    cmdB.beginRendering(&renderingInfo);

    vk::Viewport viewport{
        0, 0, (float)renderingInfo.renderArea.extent.width, (float)renderingInfo.renderArea.extent.height, 0.0f, 1.0f};
    cmdB.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{{0, 0},
                        {(uint32_t)renderingInfo.renderArea.extent.width, (uint32_t)renderingInfo.renderArea.extent.height}};
    cmdB.setScissor(0, 1, &scissors);
    cmdB.setStencilTestEnable(false);


    m_toneMappingEffect->BindPipeline(cmdB);
    m_toneMappingEffect->BindDescriptorSet(cmdB, currentIndex, 0);

    ToneMappingParameters pc;
    pc = *postProcessingContext.toneMappingParameters;

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_toneMappingEffect->GetPipelineLayout();
    pcInfo.size       = sizeof(ToneMappingParameters);
    pcInfo.offset     = 0;
    pcInfo.pValues    = &pc;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_toneMappingEffect->CmdPushConstant(cmdB, pcInfo);


    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();

    m_toneMapOutput->TransitionAttachments(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                           vk::ImageLayout::eColorAttachmentOptimal);
}

void PostProcessingSystem::AutoExposure(int                                   currentIndex,
                                        VulkanCore::VCommandBuffer&           commandBuffer,
                                        VulkanStructs::PostProcessingContext& postProcessingContext)
{
    float w = postProcessingContext.sceneRender->GetImageInfo().width;
    float h = postProcessingContext.sceneRender->GetImageInfo().height;

    LuminanceHistogramParameters pc;
    pc        = *postProcessingContext.luminanceHistrogramParameters;
    pc.width  = w;
    pc.height = h;

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout = m_luminanceHistrogram->GetPipelineLayout();
    pcInfo.size = sizeof(LuminanceHistogramParameters) - 2 * sizeof(float);  // one parameter is not taken into the account
    pcInfo.offset     = 0;
    pcInfo.pValues    = &pc;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_luminanceHistrogram->BindPipeline(commandBuffer.GetCommandBuffer());
    m_luminanceHistrogram->BindDescriptorSet(commandBuffer.GetCommandBuffer(), currentIndex, 0);
    m_luminanceHistrogram->CmdPushConstant(commandBuffer.GetCommandBuffer(), pcInfo);


    commandBuffer.GetCommandBuffer().dispatch(w / 16, h / 16, 1);

    //=================================================
    // Average luminance
    //=================================================
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_averageLuminanceOutput, vk::ImageLayout::eGeneral,
                                                    vk::ImageLayout::eShaderReadOnlyOptimal, commandBuffer.GetCommandBuffer());
    LuminanceHistogramAverageParameters pc2;
    pc2                   = *postProcessingContext.luminanceAverageParameters;
    pc2.pixelCount        = postProcessingContext.sceneRender->GetPixelCount();
    pc2.logLuminanceRange = postProcessingContext.luminanceHistrogramParameters->logRange;
    pc2.minLogLuminance   = postProcessingContext.luminanceHistrogramParameters->minLogLuminance;
    pc2.timeDelta         = postProcessingContext.deltaTime;

    pcInfo.layout     = m_averageLuminanceEffect->GetPipelineLayout();
    pcInfo.size       = sizeof(LuminanceHistogramAverageParameters);  // one parameter is not taken into the account
    pcInfo.offset     = 0;
    pcInfo.pValues    = &pc2;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_averageLuminanceEffect->BindPipeline(commandBuffer.GetCommandBuffer());
    m_averageLuminanceEffect->BindDescriptorSet(commandBuffer.GetCommandBuffer(), currentIndex, 0);
    m_averageLuminanceEffect->CmdPushConstant(commandBuffer.GetCommandBuffer(), pcInfo);

    commandBuffer.GetCommandBuffer().dispatch(1, 1, 1);

    VulkanUtils::RecordImageTransitionLayoutCommand(*m_averageLuminanceOutput, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                    vk::ImageLayout::eGeneral, commandBuffer.GetCommandBuffer());
}


void PostProcessingSystem::LensFlare(int                                   currentIndex,
                                     VulkanCore::VCommandBuffer&           commandBuffer,
                                     VulkanStructs::PostProcessingContext& postProcessingContext)
{
    m_lensFlareOutput->TransitionAttachments(commandBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                             vk::ImageLayout::eShaderReadOnlyOptimal);

    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {m_lensFlareOutput->GenerateAttachmentInfo(
        vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)};

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = renderingOutputs.size();
    renderingInfo.pColorAttachments    = renderingOutputs.data();
    renderingInfo.pDepthAttachment     = nullptr;

    auto& cmdB = commandBuffer.GetCommandBuffer();

    cmdB.beginRendering(&renderingInfo);

    vk::Viewport viewport{
        0, 0, (float)renderingInfo.renderArea.extent.width, (float)renderingInfo.renderArea.extent.height, 0.0f, 1.0f};
    cmdB.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{{0, 0},
                        {(uint32_t)renderingInfo.renderArea.extent.width, (uint32_t)renderingInfo.renderArea.extent.height}};
    cmdB.setScissor(0, 1, &scissors);
    cmdB.setStencilTestEnable(false);

    m_lensFlareEffect->BindPipeline(cmdB);
    m_lensFlareEffect->BindDescriptorSet(cmdB, currentIndex, 0);


    LensFlareParameters pc;
    pc = *postProcessingContext.lensFlareParameters;

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_toneMappingEffect->GetPipelineLayout();
    pcInfo.size       = sizeof(LensFlareParameters);
    pcInfo.offset     = 0;
    pcInfo.pValues    = &pc;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_lensFlareEffect->CmdPushConstant(cmdB, pcInfo);


    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();

    m_lensFlareOutput->TransitionAttachments(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                             vk::ImageLayout::eColorAttachmentOptimal);
}

void ToneMappingAverageLuminance(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext)
{
}

void PostProcessingSystem::Destroy()
{
    m_toneMapOutput->Destroy();
    m_lensFlareOutput->Destroy();
    m_averageLuminanceOutput->Destroy();
}


}  // namespace Renderer