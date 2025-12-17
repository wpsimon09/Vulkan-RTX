//
// Created by wpsimon09 on 17/08/2025.
//

#include "GBufferPass.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/RenderPass.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include <exception>
#include <vulkan/vulkan_enums.hpp>

namespace Renderer {
GBufferPass::GBufferPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height)
    : RenderPass(device, width, height)
{

    //==============================================
    // Generate effect
    m_gBufferEffect = effectLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::DepthPrePass);

    //===============================================
    // Generate depth pre-pass attachment
    Renderer::RenderTarget2CreatInfo depthPrepassOutputCI{width,
                                                          height,
                                                          true,
                                                          true,
                                                          m_device.GetDepthFormat(),
                                                          vk::ImageLayout::eDepthStencilReadOnlyOptimal,
                                                          vk::ResolveModeFlagBits::eMin,
                                                          false,
                                                          "Depth pre pass attachment"};

    m_depthBuffer = std::make_unique<Renderer::RenderTarget2>(m_device, depthPrepassOutputCI);

    //===============================================
    // Generate GBuffer attachments
    Renderer::RenderTarget2CreatInfo gBufferAttachmentCI{width,
                                                         height,
                                                         true,
                                                         false,
                                                         vk::Format::eR16G16B16A16Sfloat,
                                                         vk::ImageLayout::eShaderReadOnlyOptimal,
                                                         vk::ResolveModeFlagBits::eAverage,
                                                         false,
                                                         "G buffer attachemnt"};
    for(int i = 0; i < m_numGBufferAttachments; i++)
    {

        gBufferAttachmentCI.imageDebugName += AttachmentToString((EGBufferAttachments)i);
        m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, gBufferAttachmentCI));
        gBufferAttachmentCI.imageDebugName = "G Buffer attachment";
    }
}

void GBufferPass::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    m_gBufferEffect->SetNumWrites(4, 10, 0);

    m_gBufferEffect->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);
    m_gBufferEffect->WriteBuffer(currentFrameIndex, 1, 0, uniformBufferManager.GetMaterialDescriptionBuffer(currentFrameIndex));
    m_gBufferEffect->WriteImageArray(currentFrameIndex, 1, 1, uniformBufferManager.GetAll2DTextureDescriptorImageInfo());
    m_gBufferEffect->WriteBuffer(currentFrameIndex, 1, 2, uniformBufferManager.GetPerObjectBuffer(currentFrameIndex));

    m_gBufferEffect->ApplyWrites(currentFrameIndex);
}

void GBufferPass::Update(int                                   currentFrame,
                         VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                         VulkanUtils::RenderContext*           renderContext,
                         VulkanStructs::PostProcessingContext* postProcessingContext)
{
    m_gBufferEffect->SetNumWrites(2, uniformBufferManager.GetAll2DTextureDescriptorImageInfo().size() + 4, 0);

    m_gBufferEffect->WriteBuffer(currentFrame, 1, 0, uniformBufferManager.GetMaterialDescriptionBuffer(currentFrame));
    m_gBufferEffect->WriteImageArray(currentFrame, 1, 1, uniformBufferManager.GetAll2DTextureDescriptorImageInfo());

    m_gBufferEffect->ApplyWrites(currentFrame);
}

void GBufferPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    int drawCallCount = 0;


    VulkanUtils::VBarrierPosition barrierPos = {
        vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderSampledRead,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite | vk::AccessFlagBits2::eColorAttachmentWrite};
    m_depthBuffer->TransitionAttachments(cmdBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                         vk::ImageLayout::eDepthStencilReadOnlyOptimal, barrierPos);

    //=====================================
    // will loop through each render target
    for(int i = 0; i < m_numGBufferAttachments; i++)
    {
        barrierPos.dstPipelineStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        barrierPos.dstData          = vk::AccessFlagBits2::eColorAttachmentWrite;
        m_renderTargets[i]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eAttachmentOptimal,
                                                  vk::ImageLayout::eShaderReadOnlyOptimal, barrierPos);
    }

    std::vector<vk::RenderingAttachmentInfo> depthPrePassColourAttachments = {
        m_renderTargets[EGBufferAttachments::Position]->GenerateAttachmentInfo(
            vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore),
        m_renderTargets[EGBufferAttachments::Normal]->GenerateAttachmentInfo(
            vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore),
        m_renderTargets[EGBufferAttachments::Albedo]->GenerateAttachmentInfo(
            vk::ImageLayout::eAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore),
        m_renderTargets[EGBufferAttachments::Arm]->GenerateAttachmentInfo(
            vk::ImageLayout::eAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore),
        m_renderTargets[EGBufferAttachments::MotionVector]->GenerateAttachmentInfo(
            vk::ImageLayout::eAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)};

    auto depthPrePassDepthAttachment =
        m_depthBuffer->GenerateAttachmentInfo(vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                              vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore);

    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = depthPrePassColourAttachments.size();
    renderingInfo.pColorAttachments    = depthPrePassColourAttachments.data();
    renderingInfo.pDepthAttachment     = &depthPrePassDepthAttachment;
    renderingInfo.pStencilAttachment   = &depthPrePassDepthAttachment;

    m_gBufferEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_gBufferEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

    //==============================================
    // START RENDER PASS
    //==============================================
    auto& cmdB = cmdBuffer.GetCommandBuffer();

    cmdB.beginRendering(&renderingInfo);

    // if there is nothing to render end the render process
    if(renderContext->drawCalls.empty())
    {
        cmdB.endRendering();
        return;
    }

    //=================================================
    // INITIAL CONFIG
    //=================================================

    auto currentVertexBuffer = renderContext->drawCalls.begin()->second.vertexData;
    auto currentIndexBuffer  = renderContext->drawCalls.begin()->second.indexData;

    vk::DeviceSize indexBufferOffset = 0;

    cmdB.bindVertexBuffers(0, {currentVertexBuffer->buffer}, {0});
    cmdB.bindIndexBuffer(currentIndexBuffer->buffer, 0, vk::IndexType::eUint32);

    //============================================
    // CONFIGURE VIEW PORT
    //===============================================
    Renderer::ConfigureViewPort(cmdB, m_width, m_height);

    //=================================================
    // RECORD OPAQUE DRAW CALLS
    //=================================================
    for(auto& drawCall : renderContext->drawCalls)
    {


        //================================================================================================
        // BIND VERTEX BUFFER ONLY IF IT HAS CHANGED
        //================================================================================================
        if(currentVertexBuffer->BufferID != drawCall.second.vertexData->BufferID)
        {
            auto firstBinding = 0;

            std::vector<vk::Buffer>     vertexBuffers = {drawCall.second.vertexData->buffer};
            std::vector<vk::DeviceSize> offsets       = {0};
            vertexBuffers                             = {drawCall.second.vertexData->buffer};
            cmdB.bindVertexBuffers(firstBinding, vertexBuffers, offsets);
            currentVertexBuffer = drawCall.second.vertexData;
        }

        if(currentIndexBuffer->BufferID != drawCall.second.indexData->BufferID)
        {
            indexBufferOffset = 0;
            cmdB.bindIndexBuffer(drawCall.second.indexData->buffer, 0, vk::IndexType::eUint32);
            currentIndexBuffer = drawCall.second.indexData;
        }

        if(drawCall.second.selected)
        {
            cmdB.setStencilTestEnable(true);
        }
        else
        {
            cmdB.setStencilTestEnable(false);
        }

        PerObjectPushConstant pc{};
        pc.indexes.x      = drawCall.second.drawCallID;
        pc.modelMatrix    = drawCall.second.modelMatrix;
        pc.prevModelMatix = drawCall.second.previousModelMatrix;

        vk::PushConstantsInfo pcInfo;
        pcInfo.layout     = m_gBufferEffect->GetPipelineLayout();
        pcInfo.size       = sizeof(PerObjectPushConstant);
        pcInfo.offset     = 0;
        pcInfo.pValues    = &pc;
        pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

        m_gBufferEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);

        cmdB.drawIndexed(drawCall.second.indexData->size / sizeof(uint32_t), 1,
                         drawCall.second.indexData->offset / static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                         drawCall.second.vertexData->offset / static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)), 0);

        drawCallCount++;
    }
    cmdB.endRendering();
    for(int i = 0; i < m_numGBufferAttachments; i++)
    {
        barrierPos = {vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite,
                      vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                      vk::AccessFlagBits2::eShaderSampledRead};
        m_renderTargets[i]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                  vk::ImageLayout::eColorAttachmentOptimal, barrierPos);
    }

    /*
    VulkanUtils::PlaceImageMemoryBarrier(m_depthBuffer->GetPrimaryImage(), cmdBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                         vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                         vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                         vk::AccessFlagBits::eDepthStencilAttachmentRead);
    */
    //m_renderingStatistics.DrawCallCount = drawCallCount;
}

RenderTarget2& GBufferPass::GetDepthAttachment()
{
    return *m_depthBuffer;
}

std::string GBufferPass::AttachmentToString(EGBufferAttachments attachment)
{
    switch(attachment)
    {
        case Renderer::EGBufferAttachments::Albedo:
            return "Albedo";
        case Renderer::EGBufferAttachments::Normal:
            return "Normal";
        case Renderer::EGBufferAttachments::Position:
            return "Position";
        case Renderer::EGBufferAttachments::Arm:
            return "Roughness,Metallnes, Ao";
        case Renderer::EGBufferAttachments::MotionVector:
            return "Velocity";
        default:
            return "undefined !";
    }
}


void GBufferPass::Destroy()
{
    RenderPass::Destroy();
    m_depthBuffer->Destroy();
    m_gBufferEffect->Destroy();
}

}  // namespace Renderer