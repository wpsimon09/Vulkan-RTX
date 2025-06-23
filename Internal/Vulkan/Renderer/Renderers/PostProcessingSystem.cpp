//
// Created by wpsimon09 on 13/06/25.
//

#include "PostProcessingSystem.h"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

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


    Utils::Logger::LogInfo("Post processing system created");
}

void PostProcessingSystem::Render(int frameIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext)
{
    ToneMapping(frameIndex, commandBuffer, postProcessingContext);
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

    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();

    m_toneMapOutput->TransitionAttachments(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                           vk::ImageLayout::eColorAttachmentOptimal);

}

void PostProcessingSystem::Destroy()
{
    m_toneMapOutput->Destroy();
}

}  // namespace Renderer