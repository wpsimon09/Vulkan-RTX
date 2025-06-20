//
// Created by wpsimon09 on 21/12/24.
//

#include "SceneRenderer.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"

#include <sys/wait.h>
#include <Vulkan/Utils/VIimageTransitionCommands.hpp>

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
SceneRenderer::SceneRenderer(const VulkanCore::VDevice&          device,
                             ApplicationCore::EffectsLibrary&    effectsLibrary,
                             VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                             int                                 width,
                             int                                 height)
    : m_device(device)

{
    Utils::Logger::LogInfo("Creating scene renderer");

    m_width  = width;
    m_height = height;
    SceneRenderer::CreateRenderTargets(nullptr);

    //==========================
    // CREATE SHADOW MAP
    //==========================
    VulkanCore::VImage2CreateInfo shadowMapCi;
    shadowMapCi.height              = height;
    shadowMapCi.width               = width;
    shadowMapCi.imageAllocationName = "Screen-Space Shadow Map";
    shadowMapCi.samples             = vk::SampleCountFlagBits::e1;
    shadowMapCi.imageUsage          = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
    shadowMapCi.layout              = vk::ImageLayout::eShaderReadOnlyOptimal;
    shadowMapCi.format              = vk::Format::eR32G32B32A32Sfloat;
    m_shadowMap                     = std::make_unique<VulkanCore::VImage2>(m_device, shadowMapCi);


    //=========================
    // CONFIGURE DEPTH PASS EFFECT
    //=========================
    m_depthPrePassEffect = effectsLibrary.effects[ApplicationCore::EEffectType::DepthPrePass];

    //=============================
    // CONFIGURE RT SHADOW MAP PASS
    //=============================
    m_rtxShadowPassEffect = effectsLibrary.effects[ApplicationCore::EEffectType::RTShadowPass];

    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        // IMPORTANT: Depth attachment is  transitioned to shader read only optimal during creation
        m_rtxShadowPassEffect->SetNumWrites(0, 1, 0);
        m_rtxShadowPassEffect->WriteImage(i, 0, 3, m_renderTargets->GetDepthDescriptorInfo(i));
        m_rtxShadowPassEffect->ApplyWrites(i);

    }


    //=============================================
    // Transition shadow map to shader read optimal
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_shadowMap, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eUndefined,
                                                    m_device.GetTransferOpsManager().GetCommandBuffer());


    Utils::Logger::LogSuccess("Scene renderer created !");
}

void SceneRenderer::Render(int                                       currentFrameIndex,
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

void SceneRenderer::DepthPrePass(int                                       currentFrameIndex,
                                 VulkanCore::VCommandBuffer&               cmdBuffer,
                                 const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    int drawCallCount = 0;


    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset              = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent              = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount                     = 1;
    renderingInfo.colorAttachmentCount           = 0;
    renderingInfo.pColorAttachments              = nullptr;
    m_renderTargets->GetDepthAttachment().loadOp = vk::AttachmentLoadOp::eClear;

    //===========================
    // TRANSITION RESOLVE IMAGE
    VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets->GetResovedDepthImage(), vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                                    vk::ImageLayout::eDepthStencilReadOnlyOptimal, cmdBuffer);
    m_renderTargets->GetDepthAttachment().resolveMode = vk::ResolveModeFlagBitsKHR::eMin;
    renderingInfo.pDepthAttachment = &m_renderTargets->GetDepthAttachment();
    renderingInfo.pStencilAttachment = &m_renderTargets->GetDepthAttachment();

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

            if (drawCall.second.selected) {
                cmdB.setStencilTestEnable(true);
            }else {
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


    VulkanUtils::PlaceImageMemoryBarrier(
        m_renderTargets->GetDepthImage(currentFrameIndex), cmdBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal,
        vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits::eDepthStencilAttachmentRead);

    m_renderingStatistics.DrawCallCount = drawCallCount;
}

void SceneRenderer::ShadowMapPass(int                                       currentFrameIndex,
                                  VulkanCore::VCommandBuffer&               cmdBuffer,
                                  const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{

    assert(cmdBuffer.GetIsRecording() && "Command buffer is not recording ! ");

    VulkanUtils::PlacePipelineBarrier(cmdBuffer, vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                      vk::PipelineStageFlagBits::eFragmentShader);

    //=========================================================================
    // Transition shadow map from shader read only optimal to render attachment
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_shadowMap, vk::ImageLayout::eColorAttachmentOptimal,
                                                    vk::ImageLayout::eShaderReadOnlyOptimal, cmdBuffer);

    //=============================================================
    // Trasition depth from render attachemnt to shader read only
    VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets->GetResovedDepthImage(), vk::ImageLayout::eDepthStencilReadOnlyOptimal,
                                                    vk::ImageLayout::eDepthStencilAttachmentOptimal, cmdBuffer);


    vk::RenderingAttachmentInfo shadowMapAttachmentInfo{};
    shadowMapAttachmentInfo.clearValue.color = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
    shadowMapAttachmentInfo.imageLayout      = vk::ImageLayout::eColorAttachmentOptimal;
    shadowMapAttachmentInfo.imageView        = m_shadowMap->GetImageView();
    shadowMapAttachmentInfo.loadOp           = vk::AttachmentLoadOp::eClear;
    shadowMapAttachmentInfo.storeOp          = vk::AttachmentStoreOp::eStore;


    std::vector<vk::RenderingAttachmentInfo> renderingOutputs = {shadowMapAttachmentInfo};

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

    VulkanUtils::RecordImageTransitionLayoutCommand(*m_shadowMap, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                    vk::ImageLayout::eColorAttachmentOptimal, cmdB);
}


void SceneRenderer::DrawScene(int currentFrameIndex, VulkanCore::VCommandBuffer& cmdBuffer, const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{

    assert(cmdBuffer.GetIsRecording() && "Command buffer is not in recording state !");
    int drawCallCount = 0;
    //==============================================
    // CREATE RENDER PASS INFO
    //==============================================
    std::vector<vk::RenderingAttachmentInfo> colourAttachments = {
        m_renderTargets->GetColourAttachmentMultiSampled(currentFrameIndex),
    };

    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = colourAttachments.size();
    renderingInfo.pColorAttachments    = colourAttachments.data();

    m_renderTargets->GetDepthAttachment().loadOp = vk::AttachmentLoadOp::eLoad;
    m_renderTargets->GetDepthAttachment().resolveMode = vk::ResolveModeFlagBits::eNone;
    renderingInfo.pDepthAttachment               = &m_renderTargets->GetDepthAttachment();
    renderingInfo.pStencilAttachment             = &m_renderTargets->GetDepthAttachment();


    //==============================================
    // START RENDER PASS
    //==============================================
    auto& cmdB = cmdBuffer.GetCommandBuffer();

    VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets->GetColourImage(currentFrameIndex),
                                                    vk::ImageLayout::eColorAttachmentOptimal,
                                                    vk::ImageLayout::eShaderReadOnlyOptimal, cmdBuffer);

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

    VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets->GetColourImage(currentFrameIndex),
                                                    vk::ImageLayout::eShaderReadOnlyOptimal,
                                                    vk::ImageLayout::eColorAttachmentOptimal, cmdBuffer);

    m_renderingStatistics.DrawCallCount = drawCallCount;
}


void SceneRenderer::CreateRenderTargets(VulkanCore::VSwapChain* swapChain)
{
    m_renderTargets = std::make_unique<Renderer::RenderTarget>(m_device, m_width, m_height);
}


void SceneRenderer::PushDrawCallId(const vk::CommandBuffer& cmdBuffer, VulkanStructs::VDrawCallData& drawCall)
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

VulkanCore::VImage2& SceneRenderer::GetRenderedImage(int currentFrame)
{
    return m_renderTargets->GetColourImage(currentFrame);  //*m_finalRender[currentFrame]; //*m_shadowMap;
}
VulkanCore::VImage2& SceneRenderer::GetShadowMapImage() {
    return *m_shadowMap;
}

vk::DescriptorImageInfo SceneRenderer::GetShadowMapDescInfo() const
{
    return m_shadowMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
}
vk::DescriptorImageInfo SceneRenderer::GetRenderedImageConst(int frame) const {
    return m_renderTargets->GetColourImage(frame).GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
}


void SceneRenderer::Destroy()
{
    m_renderTargets->Destroy();
    m_depthPrePassEffect->Destroy();
    m_shadowMap->Destroy();
}
}  // namespace Renderer
