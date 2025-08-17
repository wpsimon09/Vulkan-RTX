//
// Created by wpsimon09 on 16/08/2025.
//

#include "VisibilityBufferPass.hpp"
#include "RenderPass.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"

namespace Renderer {
VisibilityBufferPass::VisibilityBufferPass(const VulkanCore::VDevice& device, VulkanCore::VDescriptorLayoutCache& descLayoutCache, int width, int height)
    : Renderer::RenderPass(device, width, height), m_aoOcclusionParameters{}
{
    //=================================================
    // create the effect
    m_rayTracedShadowEffect = std::make_unique<VulkanUtils::VRasterEffect>(device, "Ray traced shadow map effect",
                                                                           "Shaders/Compiled/RTShadowPass.vert.spv",
                                                                           "Shaders/Compiled/RTShadowPass.frag.spv",
                                                                           descLayoutCache, EShaderBindingGroup::ShadowRT);
    m_rayTracedShadowEffect->SetDisableDepthTest()
        .DisableStencil()
        .SetCullNone()
        .SetNullVertexBinding()
        .SetColourOutputFormat(vk::Format::eR16G16B16A16Sfloat)
        .SetPiplineNoMultiSampling();

    m_rayTracedShadowEffect->BuildEffect();

    //===================================================
    // create render target

    //==================
    // Shadow map
    Renderer::RenderTarget2CreatInfo shadowMapCI{
        width,
        height,
        false,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eNone,
    };

    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, shadowMapCI));

}
void VisibilityBufferPass::Init(int frameIndex, VulkanUtils::VUniformBufferManager&  uniformBufferManager,
                           VulkanUtils::VRayTracingDataManager& rayTracingDataManager,
                           VulkanUtils::RenderContext*          renderContext)
{

        m_rayTracedShadowEffect->SetNumWrites(3, 4, 1);

        m_rayTracedShadowEffect->WriteBuffer(frameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[frameIndex]);

        m_rayTracedShadowEffect->WriteBuffer(frameIndex, 0, 1, uniformBufferManager.GetLightBufferDescriptorInfo()[frameIndex]);

        m_rayTracedShadowEffect->WriteAccelerationStrucutre(frameIndex, 0, 2, rayTracingDataManager.GetTLAS());

        m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 3, renderContext->positionMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerDepth));

        m_rayTracedShadowEffect->WriteImage(
            frameIndex, 0, 4, MathUtils::LookUpTables.BlueNoise1024->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

        m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 5, renderContext->normalMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerDepth));


        m_rayTracedShadowEffect->ApplyWrites(frameIndex);

}

void VisibilityBufferPass::Update(int                                   currentFrame,
                             VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                             VulkanUtils::VRayTracingDataManager&  rayTracingDataManager,
                             VulkanUtils::RenderContext*           renderContext,
                             VulkanStructs::PostProcessingContext* postProcessingContext)
{
    m_rayTracedShadowEffect->SetNumWrites(0, 0, 1 );
    m_rayTracedShadowEffect->WriteAccelerationStrucutre(currentFrame, 0, 2, rayTracingDataManager.GetTLAS());
    m_rayTracedShadowEffect->ApplyWrites(currentFrame);

    m_aoOcclusionParameters = uniformBufferManager.GetApplicationState()->GetAoOcclusionParameters();
}

void VisibilityBufferPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer,  VulkanUtils::RenderContext* renderContext) {

    VulkanUtils::PlacePipelineBarrier(cmdBuffer, vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                      vk::PipelineStageFlagBits::eFragmentShader);

    //=========================================================================
    // Transition shadow map from shader read only optimal to render attachment
    m_renderTargets[EVisibilityBufferAttachments::VisibilityBuffer]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                              vk::ImageLayout::eShaderReadOnlyOptimal);


    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {m_renderTargets[EVisibilityBufferAttachments::VisibilityBuffer]->GenerateAttachmentInfo(
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

    m_rayTracedShadowEffect->BindPipeline(cmdB);
    m_rayTracedShadowEffect->BindDescriptorSet(cmdB, currentFrame, 0);

    //===========================================
    // ambient occlusion parrameters

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_rayTracedShadowEffect->GetPipelineLayout();
    pcInfo.size       = sizeof(AoOcclusionParameters);
    pcInfo.offset     = 0;
    pcInfo.pValues    = &m_aoOcclusionParameters;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_rayTracedShadowEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);


    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();

    m_renderTargets[EVisibilityBufferAttachments::VisibilityBuffer]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                              vk::ImageLayout::eColorAttachmentOptimal);
}

}  // namespace