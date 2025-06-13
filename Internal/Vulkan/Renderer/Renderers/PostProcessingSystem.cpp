//
// Created by wpsimon09 on 13/06/25.
//

#include "PostProcessingSystem.h"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
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
    VulkanCore::VImage2CreateInfo toneMapOutputCi;
    toneMapOutputCi.height              = height;
    toneMapOutputCi.width               = width;
    toneMapOutputCi.imageAllocationName = "Final render";
    toneMapOutputCi.samples             = vk::SampleCountFlagBits::e1;
    toneMapOutputCi.imageUsage          = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
    toneMapOutputCi.layout              = vk::ImageLayout::eShaderReadOnlyOptimal;
    toneMapOutputCi.format              = vk::Format::eR8G8B8A8Srgb;

    m_toneMapResult.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {

        m_toneMapResult[i] = std::make_unique<VulkanCore::VImage2>(device, toneMapOutputCi);
        VulkanUtils::RecordImageTransitionLayoutCommand(*m_toneMapResult[i], vk::ImageLayout::eShaderReadOnlyOptimal,
                                                        vk::ImageLayout::eUndefined,
                                                        m_device.GetTransferOpsManager().GetCommandBuffer());
    }

    Utils::Logger::LogInfo("Post processing system created");
}
void PostProcessingSystem::Render(int frameIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext)
{
    ToneMapping(frameIndex, commandBuffer, postProcessingContext);
}

void PostProcessingSystem::ToneMapping(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext) {
    //=================================================================
    // Transition tone mapping output to the output attachment optimal
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_toneMapResult[currentIndex], vk::ImageLayout::eColorAttachmentOptimal,
                                                    vk::ImageLayout::eShaderReadOnlyOptimal, commandBuffer.GetCommandBuffer());


    vk::RenderingAttachmentInfo toneMapAttachmentInfo{};
    toneMapAttachmentInfo.clearValue.color = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
    toneMapAttachmentInfo.imageLayout      = vk::ImageLayout::eColorAttachmentOptimal;
    toneMapAttachmentInfo.imageView        = m_toneMapResult[currentIndex]->GetImageView();
    toneMapAttachmentInfo.loadOp           = vk::AttachmentLoadOp::eClear;
    toneMapAttachmentInfo.storeOp          = vk::AttachmentStoreOp::eStore;


    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {toneMapAttachmentInfo};

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

    VulkanUtils::RecordImageTransitionLayoutCommand(*m_toneMapResult[currentIndex], vk::ImageLayout::eShaderReadOnlyOptimal,
                                                    vk::ImageLayout::eColorAttachmentOptimal, cmdB);

}

void PostProcessingSystem::Destroy()
{
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_toneMapResult[i]->Destroy();
    }
}

}  // namespace Renderer