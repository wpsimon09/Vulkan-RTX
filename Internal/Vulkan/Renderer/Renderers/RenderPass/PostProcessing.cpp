//
// Created by wpsimon09 on 21/08/2025.
//

#include "PostProcessing.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
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
}  // namespace Renderer