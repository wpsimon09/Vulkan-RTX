//
// Created by wpsimon09 on 17/08/2025.
//

#include "GBufferPass.hpp"

#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

namespace Renderer {
GBufferPass::GBufferPass(const VulkanCore::VDevice& device, VulkanCore::VDescriptorLayoutCache& descLayoutCache, int width, int height)
    : RenderPass(device, width, height)
{

    //==============================================
    // Generate effect
    m_gBufferEffect = std::make_unique<VulkanUtils::VRasterEffect>(device, "Depth-PrePass effect", "Shaders/Compiled/DepthPrePass.vert.spv",
                                                                   "Shaders/Compiled/DepthPrePass.frag.spv", descLayoutCache,
                                                                   EShaderBindingGroup::ForwardUnlitNoMaterial, 2);
    m_gBufferEffect->SetVertexInputMode(EVertexInput::Position_Normal).SetDepthOpLess();
    m_gBufferEffect->AddColourAttachmentFormat(vk::Format::eR16G16B16A16Sfloat);

    m_gBufferEffect->BuildEffect();

    //===============================================
    // Generate depth pre-pass attachment
    Renderer::RenderTarget2CreatInfo depthPrepassOutputCI{
        width,
        height,
        true,
        true,
        m_device.GetDepthFormat(),
        vk::ImageLayout::eDepthStencilReadOnlyOptimal,
        vk::ResolveModeFlagBits::eMin,
    };

    m_depthBuffer = std::make_unique<Renderer::RenderTarget2>(m_device, depthPrepassOutputCI);

    //===============================================
    // Generate GBuffer attachments
    Renderer::RenderTarget2CreatInfo gBufferAttachmentCI{
        width,
        height,
        true,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eAverage,
    };
    for(int i = 0; i < m_numGBufferAttachments; i++)
    {
        m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device,gBufferAttachmentCI));
    }
}

void GBufferPass::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    m_gBufferEffect->SetNumWrites(3, 0, 0);
    m_gBufferEffect->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);

    m_gBufferEffect->ApplyWrites(currentFrameIndex);
}

void GBufferPass::Update(int                                   currentFrame,
                         VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                         VulkanUtils::RenderContext*           renderContext,
                         VulkanStructs::PostProcessingContext* postProcessingContext)
{
}

void GBufferPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{

}

RenderTarget2& GBufferPass::GetDepthAttachment() {
    return *m_depthBuffer;
}

}  // namespace Renderer