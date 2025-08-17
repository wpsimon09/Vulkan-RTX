//
// Created by wpsimon09 on 21/12/24.
//

#include "ForwardRenderer.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"

#include <memory>
#include <sys/wait.h>
#include <Vulkan/Utils/VIimageTransitionCommands.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "RenderPass/VisibilityBufferPass.hpp"
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
#include "Vulkan/Renderer/Renderers/RenderPass/VisibilityBufferPass.hpp"


namespace Renderer {
ForwardRenderer::ForwardRenderer(const VulkanCore::VDevice&          device,
                                 VulkanUtils::RenderContext*         renderContext,
                                 ApplicationCore::EffectsLibrary&    effectsLibrary,
                                 VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                                 int                                 width,
                                 int                                 height)
    : m_device(device), m_renderContextPtr(renderContext)

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

    //=============================
    // CONFIGURE BILATERIAL PASS
    //=============================
    m_bilateralDenoiser =
        std::make_unique<VulkanUtils::VComputeEffect>(m_device, "BilaterialPass", "Shaders/Compiled/Bilaterial-Filter.spv",
                                                      descLayoutCache, EShaderBindingGroup::ComputePostProecess);
    m_bilateralDenoiser->BuildEffect();

    m_bilateralDenoiser->GetReflectionData()->Print();

    VulkanCore::VImage2CreateInfo m_visiblityBuffer_DenoisedCI;
    m_visiblityBuffer_DenoisedCI.width     = m_width;
    m_visiblityBuffer_DenoisedCI.height    = m_height;
    m_visiblityBuffer_DenoisedCI.isStorage = true;
    m_visiblityBuffer_DenoisedCI.format    = vk::Format::eR16G16B16A16Sfloat;
    m_visiblityBuffer_DenoisedCI.layout    = vk::ImageLayout::eGeneral;
    m_visiblityBuffer_DenoisedCI.imageUsage |= vk::ImageUsageFlagBits::eStorage;

    m_visiblityBuffer_Denoised = std::make_unique<VulkanCore::VImage2>(m_device, m_visiblityBuffer_DenoisedCI);

    VulkanUtils::RecordImageTransitionLayoutCommand(*m_visiblityBuffer_Denoised, vk::ImageLayout::eGeneral, vk::ImageLayout::eUndefined,
                                                    m_device.GetTransferOpsManager().GetCommandBuffer());
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
        vk::ImageLayout::eDepthStencilReadOnlyOptimal,
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
    // Normal map
    m_normalBufferOutput = std::make_unique<Renderer::RenderTarget2>(m_device, positionBufferCI);


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

    m_fogPassOutput = std::make_unique<Renderer::RenderTarget2>(m_device, fogPassOutputCI);

    m_renderContextPtr->normalMap = &m_normalBufferOutput->GetResolvedImage();
    m_renderContextPtr->positionMap =&m_positionBufferOutput->GetResolvedImage();

    m_visibilityBufferPass = std::make_unique<Renderer::VisibilityBufferPass>(device, descLayoutCache, width, height);


    Utils::Logger::LogSuccess("Scene renderer created !");
}
void ForwardRenderer::Init(int frameIndex,
                            VulkanUtils::VUniformBufferManager&  uniformBufferManager,
                           VulkanUtils::VRayTracingDataManager& rayTracingDataManager,
                           VulkanUtils::RenderContext*          renderContext)
{
    m_visibilityBufferPass->Init(frameIndex ,uniformBufferManager, rayTracingDataManager, renderContext);
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

    //===========================
    // denoise the shadow pass
    DenoiseVisibility(currentFrameIndex, cmdBuffer, uniformBufferManager);

    //============================
    // uses forward renderer to render the scene
    DrawScene(currentFrameIndex, cmdBuffer, uniformBufferManager);

    //==================================
    // render the fog if it is in scene
    if(m_postProcessingFogVolumeDrawCall)
    {
        PostProcessingFogPass(currentFrameIndex, cmdBuffer, uniformBufferManager);
    }



    m_frameCount++;
}
VulkanCore::VImage2* ForwardRenderer::GetForwardRendererResult() const
{
    return m_forwardRendererOutput;
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
    return m_visibilityBufferPass->GetRenderTarget();
}
Renderer::RenderTarget2& ForwardRenderer::GetLightPassOutput() const
{
    return *m_lightingPassOutput;
}

Renderer::RenderTarget2& ForwardRenderer::GetNormalBufferOutput() const
{
    return *m_normalBufferOutput;
}

VulkanCore::VImage2& ForwardRenderer::GetDenoisedVisibilityBuffer() const
{
    return *m_visiblityBuffer_Denoised;
}

void ForwardRenderer::DepthPrePass(int                                       currentFrameIndex,
                                   VulkanCore::VCommandBuffer&               cmdBuffer,
                                   const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    int drawCallCount = 0;


    m_depthPrePassOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                                vk::ImageLayout::eDepthStencilReadOnlyOptimal);

    //===========================
    // TRANSITION POSITION BUFFER
    m_positionBufferOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                                  vk::ImageLayout::eShaderReadOnlyOptimal);

    m_normalBufferOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                                vk::ImageLayout::eShaderReadOnlyOptimal);

    std::vector<vk::RenderingAttachmentInfo> depthPrePassColourAttachments = {
        m_positionBufferOutput->GenerateAttachmentInfo(vk::ImageLayout::eColorAttachmentOptimal,
                                                       vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore),
        m_normalBufferOutput->GenerateAttachmentInfo(vk::ImageLayout::eColorAttachmentOptimal,
                                                     vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)};

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
    Renderer::ConfigureViewPort(cmdB, m_width, m_height);

    //=================================================
    // RECORD OPAQUE DRAW CALLS
    //=================================================
    for(auto& drawCall : m_renderContextPtr->drawCalls)
    {
        if(drawCall.second.postProcessingEffect)
        {
            m_postProcessingFogVolumeDrawCall = &drawCall.second;
            continue;
        }
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

    m_positionBufferOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                  vk::ImageLayout::eColorAttachmentOptimal);

    m_normalBufferOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                vk::ImageLayout::eColorAttachmentOptimal);

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
    m_visibilityBufferPass->Render(currentFrameIndex, cmdBuffer, m_renderContextPtr);
}

//==================================================================
// De noiser pass, applies bilateral filter to the visibility buffer

void ForwardRenderer::DenoiseVisibility(int                                       currentFrameIndex,
                                        VulkanCore::VCommandBuffer&               cmdBuffer,
                                        const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    assert(cmdBuffer.GetIsRecording() && " Command buffer is not in recording state");

    VulkanUtils::PlaceImageMemoryBarrier(*m_visiblityBuffer_Denoised, cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                         vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                         vk::PipelineStageFlagBits::eComputeShader,
                                         vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderWrite);

    m_bilateralDenoiser->SetNumWrites(1, 4, 0);

    //m_bilateralDenoiser->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);

    m_bilateralDenoiser->WriteImage(currentFrameIndex, 0, 1,
                                    m_visibilityBuffer->GetPrimaryImage().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    m_bilateralDenoiser->WriteImage(currentFrameIndex, 0, 2, m_visiblityBuffer_Denoised->GetDescriptorImageInfo());

    m_bilateralDenoiser->WriteImage(currentFrameIndex, 0, 3,
                                    m_normalBufferOutput->GetResolvedImage().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

    m_bilateralDenoiser->ApplyWrites(currentFrameIndex);

    m_bilateralDenoiser->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_bilateralDenoiser->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrameIndex, 0);

    BilaterialFilterParameters pc = uniformBufferManager.GetApplicationState()->GetBilateralFilaterParameters();

    pc.width  = m_visiblityBuffer_Denoised->GetImageInfo().width;
    pc.height = m_visiblityBuffer_Denoised->GetImageInfo().height;

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_bilateralDenoiser->GetPipelineLayout();
    pcInfo.size       = sizeof(BilaterialFilterParameters);
    pcInfo.offset     = 0;
    pcInfo.pValues    = &pc;
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_bilateralDenoiser->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);

    cmdBuffer.GetCommandBuffer().dispatch(m_visiblityBuffer_Denoised->GetImageInfo().width / 16,
                                          m_visiblityBuffer_Denoised->GetImageInfo().height / 16, 1);

    VulkanUtils::PlaceImageMemoryBarrier(*m_visiblityBuffer_Denoised, cmdBuffer, vk::ImageLayout::eGeneral,
                                         vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eComputeShader,
                                         vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eShaderWrite,
                                         vk::AccessFlagBits::eShaderRead);
}


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
                                                     vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore);

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
    Renderer::ConfigureViewPort(cmdB, m_width, m_height);

    //=================================================
    // RECORD OPAQUE DRAW CALLS
    //=================================================
    currentEffect->BindPipeline(cmdB);
    currentEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrameIndex, 0);

    for(auto& drawCall : m_renderContextPtr->drawCalls)
    {
        if(drawCall.second.postProcessingEffect)
        {
            continue;
        }
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


        m_forwardRendererOutput = &m_lightingPassOutput->GetResolvedImage();

        drawCallCount++;
    }

    cmdB.endRendering();


    m_lightingPassOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                vk::ImageLayout::eColorAttachmentOptimal);

    m_depthPrePassOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eDepthStencilReadOnlyOptimal,
                                                vk::ImageLayout::eDepthStencilAttachmentOptimal);

    m_renderingStatistics.DrawCallCount = drawCallCount;
}
void ForwardRenderer::PostProcessingFogPass(int                                       currentFrameIndex,
                                            VulkanCore::VCommandBuffer&               cmdBuffer,
                                            const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    // this might not be the best thing to do but for now it should suffice
    m_fogPassOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);


    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {m_fogPassOutput->GenerateAttachmentInfo(
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

    m_postProcessingFogVolumeDrawCall->effect->BindPipeline(cmdB);
    m_postProcessingFogVolumeDrawCall->effect->BindDescriptorSet(cmdB, currentFrameIndex, 0);

    cmdB.draw(3, 1, 0, 0);

    cmdB.endRendering();

    m_fogPassOutput->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);

    m_postProcessingFogVolumeDrawCall = nullptr;

    m_forwardRendererOutput = &m_fogPassOutput->GetPrimaryImage();
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
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    drawCall.effect->CmdPushConstant(cmdBuffer, pcInfo);
}


void ForwardRenderer::Destroy()
{
    //m_renderTargets->Destroy();
    m_depthPrePassOutput->Destroy();
    m_visibilityBuffer->Destroy();
    m_lightingPassOutput->Destroy();
    m_positionBufferOutput->Destroy();
    m_fogPassOutput->Destroy();
    m_normalBufferOutput->Destroy();
    m_visiblityBuffer_Denoised->Destroy();
    m_visibilityBufferPass->Destroy();
    //m_shadowMap->Destroy();
}
}  // namespace Renderer
