//
// Created by wpsimon09 on 21/12/24.
//

#include "ForwardRenderer.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"

#include <sys/wait.h>
#include <Vulkan/Utils/VIimageTransitionCommands.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"


namespace Renderer {
ForwardRenderer::ForwardRenderer(const VulkanCore::VDevice&          device,
                                 ApplicationCore::EffectsLibrary&    effectsLibrary,
                                 VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                                 int                                 width,
                                 int                                 height)
    : m_device(device)

{
    Utils::Logger::LogInfo("Creating scene renderer");

    m_width  = width;
    m_height = height;

    //=========================
    // CONFIGURE DEPTH PASS EFFECT
    //=========================
    m_depthPrePassEffect = effectsLibrary.effects[ApplicationCore::EEffectType::DepthPrePass];

    //=============================
    // CONFIGURE RT SHADOW MAP PASS
    //=============================
    m_rtxShadowPassEffect = effectsLibrary.effects[ApplicationCore::EEffectType::RTShadowPass];

    //===========================================
    //===== CREATE RENDER TARGETS
    //===========================================

    //=============
    //depth prepass
    Renderer::RenderTarget2CreatInfo depthPrepassOutputCI{
        width,
        height,
        true,
        true,
        m_device.GetDepthFormat(),
        vk::ImageLayout::eDepthStencilAttachmentOptimal,
        vk::ResolveModeFlagBits::eMin,
    };

    m_depthPrePassOutput = std::make_unique<Renderer::RenderTarget2>(m_device, depthPrepassOutputCI);

    //=================
    // position buffer
    Renderer::RenderTarget2CreatInfo positionBufferCI{
        width,
        height,
        true,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eAverage,
    };

    m_positionBufferOutput = std::make_unique<Renderer::RenderTarget2>(m_device, positionBufferCI);

    //==================
    // Shadow map
    Renderer::RenderTarget2CreatInfo shadowMapCI{
        width,
        height,
        false,
        false,
        vk::Format::eR32Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eNone,
    };

    m_visibilityBuffer = std::make_unique<Renderer::RenderTarget2>(m_device, shadowMapCI);

    //==================
    // Lightning pass
    Renderer::RenderTarget2CreatInfo lightPassCI{
        width,
        height,
        true,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eAverage,
    };

    m_lightingPassOutput = std::make_unique<Renderer::RenderTarget2>(m_device, lightPassCI);

    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        // IMPORTANT: Depth attachment is  transitioned to shader read only optimal during creation
        m_rtxShadowPassEffect->SetNumWrites(0, 1, 0);

        m_rtxShadowPassEffect->WriteImage(
            i, 0, 3, m_positionBufferOutput->GetResolvedImage().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
        m_rtxShadowPassEffect->ApplyWrites(i);
    }




    Utils::Logger::LogSuccess("Scene renderer created !");
}

void ForwardRenderer::Render(int                                       currentFrameIndex,
                             VulkanCore::VCommandBuffer&               cmdBuffer,
                             const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                             VulkanUtils::RenderContext*               renderContext)
{

    m_renderContextPtr = renderContext;
    //=====================================================
    // RECORD COMMAND BUFFER
    //=====================================================
    assert(cmdBuffer.GetIsRecording() && "Command buffer is not in recording state !");

    // descriptor set 0 is allways the samme

    //============================
    // generates depth buffer
    DepthPrePass(currentFrameIndex, cmdBuffer, uniformBufferManager);

    //===========================
    // generates shadow mapp in  screen space
    ShadowMapPass(currentFrameIndex, cmdBuffer, uniformBufferManager);

    //============================
    // uses forward renderer to render the scene
    DrawScene(currentFrameIndex, cmdBuffer, uniformBufferManager);

    m_frameCount++;
}
Renderer::RenderTarget2& ForwardRenderer::GetDepthPrePassOutput() const
{
    return *m_depthPrePassOutput;
}
Renderer::RenderTarget2& ForwardRenderer::GetPositionBufferOutput() const
{
    return *m_positionBufferOutput;
}
Renderer::RenderTarget2& ForwardRenderer::GetShadowMapOutput() const
{
    return *m_visibilityBuffer;
}
Renderer::RenderTarget2& ForwardRenderer::GetLightPassOutput() const
{
    return *m_lightingPassOutput;
}

void ForwardRenderer::DepthPrePass(int                                       currentFrameIndex,
                                   VulkanCore::VCommandBuffer&               cmdBuffer,
                                   const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    int drawCallCount = 0;

    std::vector<vk::RenderingAttachmentInfo> depthPrePassColourAttachments = {m_positionBufferOutput->GenerateAttachmentInfo(
        vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)};

    auto depthPrePassDepthAttachment =
        m_depthPrePassOutput->GenerateAttachmentInfo(vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                                     vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore);

    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = depthPrePassColourAttachments.size();
    renderingInfo.pColorAttachments    = depthPrePassColourAttachments.data();
    renderingInfo.pDepthAttachment     = &depthPrePassDepthAttachment;
    renderingInfo.pStencilAttachment   = &depthPrePassDepthAttachment;

    //===========================
    // TRANSITION POSITION BUFFER
    m_positionBufferOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                                  vk::ImageLayout::eShaderReadOnlyOptimal);


    m_depthPrePassEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_depthPrePassEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrameIndex, 0);

    //==============================================
    // START RENDER PASS
    //==============================================
    auto& cmdB = cmdBuffer.GetCommandBuffer();

    cmdB.beginRendering(&renderingInfo);

    // if there is nothing to render end the render process
    if(m_renderContextPtr->drawCalls.empty())
    {
        cmdB.endRendering();
        m_renderingStatistics.DrawCallCount = 0;
        return;
    }

    //=================================================
    // INITIAL CONFIG
    //=================================================

    auto currentVertexBuffer = m_renderContextPtr->drawCalls.begin()->second.vertexData;
    auto currentIndexBuffer  = m_renderContextPtr->drawCalls.begin()->second.indexData;

    vk::DeviceSize indexBufferOffset = 0;

    cmdB.bindVertexBuffers(0, {currentVertexBuffer->buffer}, {0});
    cmdB.bindIndexBuffer(currentIndexBuffer->buffer, 0, vk::IndexType::eUint32);

    //============================================
    // CONFIGURE VIEW PORT
    //===============================================
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;

    viewport.width  = static_cast<float>(m_width);
    viewport.height = static_cast<float>(m_height);

    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmdB.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{};
    scissors.offset.x      = 0;
    scissors.offset.y      = 0;
    scissors.extent.width  = m_width;
    scissors.extent.height = m_height;

    cmdB.setScissor(0, 1, &scissors);

    //=================================================
    // RECORD OPAQUE DRAW CALLS
    //=================================================
    for(auto& drawCall : m_renderContextPtr->drawCalls)
    {
        if(drawCall.second.inDepthPrePass)
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

            PushDrawCallId(cmdB, drawCall.second);

            cmdB.drawIndexed(drawCall.second.indexData->size / sizeof(uint32_t), 1,
                             drawCall.second.indexData->offset / static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                             drawCall.second.vertexData->offset / static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)), 0);

            drawCallCount++;
        }
    }
    cmdB.endRendering();

    m_positionBufferOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);

    VulkanUtils::PlaceImageMemoryBarrier(
        m_depthPrePassOutput->GetPrimaryImage(), cmdBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal,
        vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits::eDepthStencilAttachmentRead);

    m_renderingStatistics.DrawCallCount = drawCallCount;
}

void ForwardRenderer::ShadowMapPass(int                                       currentFrameIndex,
                                    VulkanCore::VCommandBuffer&               cmdBuffer,
                                    const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{

    assert(cmdBuffer.GetIsRecording() && "Command buffer is not recording ! ");

    VulkanUtils::PlacePipelineBarrier(cmdBuffer, vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                      vk::PipelineStageFlagBits::eFragmentShader);

    //=========================================================================
    // Transition shadow map from shader read only optimal to render attachment
    m_visibilityBuffer->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                              vk::ImageLayout::eShaderReadOnlyOptimal);


    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {m_visibilityBuffer->GenerateAttachmentInfo(
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

    m_rtxShadowPassEffect->BindPipeline(cmdB);
    m_rtxShadowPassEffect->BindDescriptorSet(cmdB, currentFrameIndex, 0);

    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();

    m_visibilityBuffer->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                              vk::ImageLayout::eColorAttachmentOptimal);
}  // namespace Renderer


void ForwardRenderer::DrawScene(int                                       currentFrameIndex,
                                VulkanCore::VCommandBuffer&               cmdBuffer,
                                const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{

    assert(cmdBuffer.GetIsRecording() && "Command buffer is not in recording state !");
    int drawCallCount = 0;
    //==============================================
    // CREATE RENDER PASS INFO
    //==============================================
    std::vector<vk::RenderingAttachmentInfo> colourAttachments = {
        m_lightingPassOutput->GenerateAttachmentInfo(vk::ImageLayout::eColorAttachmentOptimal,
                                                     vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore),
    };

    auto depthAttachment =
        m_depthPrePassOutput->GenerateAttachmentInfo(vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                                     vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eDontCare);

    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = colourAttachments.size();
    renderingInfo.pColorAttachments    = colourAttachments.data();

    renderingInfo.pDepthAttachment   = &depthAttachment;
    renderingInfo.pStencilAttachment = &depthAttachment;


    //==============================================
    // START RENDER PASS
    //==============================================
    auto& cmdB = cmdBuffer.GetCommandBuffer();


    m_lightingPassOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                                vk::ImageLayout::eShaderReadOnlyOptimal);


    cmdB.beginRendering(&renderingInfo);


    // if there is nothing to render end the render process
    if(m_renderContextPtr->drawCalls.empty())
    {
        cmdB.endRendering();
        m_renderingStatistics.DrawCallCount = drawCallCount;
        return;
    }
    //=================================================
    // UPDATE DESCRIPTOR SETS
    //=================================================

    auto  currentVertexBuffer = m_renderContextPtr->drawCalls.begin()->second.vertexData;
    auto  currentIndexBuffer  = m_renderContextPtr->drawCalls.begin()->second.indexData;
    auto& currentEffect       = m_renderContextPtr->drawCalls.begin()->second.effect;

    vk::DeviceSize indexBufferOffset = 0;

    cmdB.bindVertexBuffers(0, {currentVertexBuffer->buffer}, {0});
    cmdB.bindIndexBuffer(currentIndexBuffer->buffer, 0, vk::IndexType::eUint32);
    //============================================
    // CONFIGURE VIEW PORT
    //===============================================
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;

    viewport.width  = static_cast<float>(m_width);
    viewport.height = static_cast<float>(m_height);

    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmdB.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{};
    scissors.offset.x      = 0;
    scissors.offset.y      = 0;
    scissors.extent.width  = m_width;
    scissors.extent.height = m_height;

    cmdB.setScissor(0, 1, &scissors);

    //=================================================
    // RECORD OPAQUE DRAW CALLS
    //=================================================
    currentEffect->BindPipeline(cmdB);
    currentEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrameIndex, 0);

    for(auto& drawCall : m_renderContextPtr->drawCalls)
    {
        if (drawCall.second.postProcessingEffect) { continue; }
        auto& material = drawCall.second.material;
        if(drawCall.second.effect != currentEffect)
        {
            currentEffect = drawCall.second.effect;
            drawCall.second.effect->BindPipeline(cmdB);
            currentEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrameIndex, 0);
        }

        if(drawCall.second.selected)
            cmdB.setStencilTestEnable(true);
        else
            cmdB.setStencilTestEnable(false);

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


        PushDrawCallId(cmdB, drawCall.second);


        cmdB.drawIndexed(drawCall.second.indexData->size / sizeof(uint32_t), 1,
                         drawCall.second.indexData->offset / static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                         drawCall.second.vertexData->offset / static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)), 0);

        drawCallCount++;
    }

    cmdB.endRendering();

    m_lightingPassOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                vk::ImageLayout::eColorAttachmentOptimal);

    m_renderingStatistics.DrawCallCount = drawCallCount;
}


void ForwardRenderer::PushDrawCallId(const vk::CommandBuffer& cmdBuffer, VulkanStructs::VDrawCallData& drawCall)
{
    PerObjectPushConstant pc{};
    pc.indexes.x   = drawCall.drawCallID;
    pc.modelMatrix = drawCall.modelMatrix;

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = drawCall.effect->GetPipelineLayout();
    pcInfo.size       = sizeof(PerObjectPushConstant);
    pcInfo.offset     = 0;
    pcInfo.pValues    = &pc;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eVertex;

    drawCall.effect->CmdPushConstant(cmdBuffer, pcInfo);
}


void ForwardRenderer::Destroy()
{
    //m_renderTargets->Destroy();
    m_depthPrePassOutput->Destroy();
    m_visibilityBuffer->Destroy();
    m_lightingPassOutput->Destroy();
    m_positionBufferOutput->Destroy();
    //m_shadowMap->Destroy();
}
}  // namespace Renderer
