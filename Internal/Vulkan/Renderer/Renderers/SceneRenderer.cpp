//
// Created by wpsimon09 on 21/12/24.
//

#include "SceneRenderer.hpp"

#include <sys/wait.h>
#include <Vulkan/Utils/VIimageTransitionCommands.hpp>

#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Utils/LinearyTransformedCosinesValues.hpp"
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
#include "Vulkan/Utils/VResrouceGroup/VDescriptorSetStructs.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"


namespace Renderer {
SceneRenderer::SceneRenderer(const VulkanCore::VDevice& device, VulkanUtils::VResourceGroupManager& pushDescriptorManager, VulkanCore::VDescriptorLayoutCache& descLayoutCache, int width, int height)
    : m_pushDescriptorManager(pushDescriptorManager)
    , m_device(device)

{
    Utils::Logger::LogInfo("Creating scene renderer");

    m_width  = width;
    m_height = height;
    SceneRenderer::CreateRenderTargets(nullptr);

    //=========================
    // CONFIGURE DEPTH PASS EFFECT
    //=========================
    m_depthPrePassEffect = std::make_unique<VulkanUtils::VRasterEffect>(
        m_device, "Depth-PrePass effect", "Shaders/Compiled/DepthPrePass.vert.spv", "Shaders/Compiled/DepthPrePass.frag.spv", descLayoutCache,
        m_pushDescriptorManager.GetResourceGroup(VulkanUtils::EDescriptorLayoutStruct::Basic));
    m_depthPrePassEffect->SetVertexInputMode(EVertexInput::PositionOnly).SetDepthOpLess();

    m_depthPrePassEffect->BuildEffect();

    Utils::Logger::LogSuccess("Scene renderer created !");
}


void SceneRenderer::PushDataToGPU(const vk::CommandBuffer&                  cmdBuffer,
                                  int                                       currentFrameIndex,
                                  int                                       objectIndex,
                                  VulkanStructs::VDrawCallData&              drawCall,
                                  const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    switch(drawCall.effect->GetLayoutStructType())
    {
        case VulkanUtils::EDescriptorLayoutStruct::Basic: {
            auto& basicEffecResourceGroup =
                std::get<VulkanUtils::BasicDescriptorSet>(drawCall.effect->GetResrouceGroupStructVariant());
            basicEffecResourceGroup.buffer1 = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];
            basicEffecResourceGroup.buffer2 =
                uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.drawCallID)[currentFrameIndex];

            drawCall.effect->SetNumWrites(2,0,0);
            drawCall.effect->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);
            drawCall.effect->WriteBuffer(currentFrameIndex, 0, 1, uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.drawCallID)[currentFrameIndex]);

            drawCall.effect->ApplyWrites(currentFrameIndex);

            break;
        }

        case VulkanUtils::EDescriptorLayoutStruct::UnlitSingleTexture: {
            auto& unlitSingleTextureResrouceGroup =
                std::get<VulkanUtils::Unlit>(drawCall.effect->GetResrouceGroupStructVariant());
            ;

            // 20 images, cause i am not sure how many there might be in the future
            drawCall.effect->SetNumWrites(2, 20, 0);
            drawCall.effect->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);
            drawCall.effect->WriteBuffer(currentFrameIndex, 0, 1, uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.drawCallID)[currentFrameIndex]);

            // cast might be required here
            drawCall.material->UpdateGPUTextureData(unlitSingleTextureResrouceGroup);

            if(drawCall.effect->GetName() == "Sky Box")
            {
                if(m_renderContextPtr->hdrCubeMap)
                {
                    unlitSingleTextureResrouceGroup.texture2D_1 =
                        m_renderContextPtr->hdrCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerClampToEdge);
                }
                else
                    unlitSingleTextureResrouceGroup.texture2D_1 =
                        m_renderContextPtr->dummyCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerClampToEdge);
            }

            cmdBuffer.pushDescriptorSetWithTemplateKHR(drawCall.effect->GetUpdateTemplate(), drawCall.effect->GetPipelineLayout(),
                                                       0, unlitSingleTextureResrouceGroup, m_device.DispatchLoader);
            break;
        }
        case VulkanUtils::EDescriptorLayoutStruct::ForwardShading: {
            auto& forwardShadingResourceGroup =
                std::get<VulkanUtils::ForwardShadingDstSet>(drawCall.effect->GetResrouceGroupStructVariant());
            forwardShadingResourceGroup.buffer1 = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];
            forwardShadingResourceGroup.buffer2 =
                uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.drawCallID)[currentFrameIndex];
            ;
            forwardShadingResourceGroup.buffer3 = uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrameIndex];

            drawCall.material->UpdateGPUTextureData(forwardShadingResourceGroup);

            forwardShadingResourceGroup.texture2D_5 =
                MathUtils::LUT.LTC->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
            forwardShadingResourceGroup.texture2D_6 =
                MathUtils::LUT.LTCInverse->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
            if(m_renderContextPtr->irradianceMap)
            {
                forwardShadingResourceGroup.texture2D_7 =
                    m_renderContextPtr->irradianceMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
            }
            else
            {
                forwardShadingResourceGroup.texture2D_7 =
                    m_renderContextPtr->dummyCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
                ;
            }

            if(m_renderContextPtr->prefilterMap)
            {
                forwardShadingResourceGroup.texture2D_8 =
                    m_renderContextPtr->prefilterMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips);
            }
            else
            {
                forwardShadingResourceGroup.texture2D_8 =
                    m_renderContextPtr->dummyCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
                ;
            }

            if(m_renderContextPtr->brdfMap)
            {
                forwardShadingResourceGroup.texture2D_9 =
                    m_renderContextPtr->brdfMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
            }
            else
            {
                forwardShadingResourceGroup.texture2D_9 =
                    m_renderContextPtr->dummyCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
                ;
            }
            //forwardShadding.texture2D_7 = m_renderContextPtr->irradianceMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

            cmdBuffer.pushDescriptorSetWithTemplateKHR(drawCall.effect->GetUpdateTemplate(), drawCall.effect->GetPipelineLayout(),
                                                       0, forwardShadingResourceGroup, m_device.DispatchLoader);
            break;
        }
    }
}

void SceneRenderer::DepthPrePass(int currentFrameIndex,VulkanCore::VCommandBuffer& cmdBuffer, const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    int drawCallCount = 0;


    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset              = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent              = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount                     = 1;
    renderingInfo.colorAttachmentCount           = 0;
    renderingInfo.pColorAttachments              = nullptr;
    m_renderTargets->GetDepthAttachment().loadOp = vk::AttachmentLoadOp::eClear;

    renderingInfo.pDepthAttachment = &m_renderTargets->GetDepthAttachment();

    m_depthPrePassEffect->BindPipeline(cmdBuffer.GetCommandBuffer());


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
    cmdB.setStencilTestEnable(true);
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


            m_depthPrePassEffect->SetNumWrites(2, 0, 0);
            m_depthPrePassEffect->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);
            m_depthPrePassEffect->WriteBuffer(currentFrameIndex, 0, 1, uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.second.drawCallID)[currentFrameIndex]);

            m_depthPrePassEffect->ApplyWrites(currentFrameIndex);


            // TODO: so wrong i will rewrite this to the descriptor indexing
            m_depthPrePassEffect->BindDescriptorSet(cmdB, currentFrameIndex, 0);

            cmdB.drawIndexed(drawCall.second.indexData->size / sizeof(uint32_t), 1,
                                  drawCall.second.indexData->offset / static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                                  drawCall.second.vertexData->offset / static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)),
                                  0);

            drawCallCount++;
        }
    }
    cmdB.endRendering();


    VulkanUtils::PlaceImageMemoryBarrier(
        m_renderTargets->GetDepthImage(currentFrameIndex), cmdBuffer,
        vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal,
        vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::AccessFlagBits::eDepthStencilAttachmentRead);

    m_renderingStatistics.DrawCallCount = drawCallCount;
}


void SceneRenderer::Render(int                                       currentFrameIndex,
                           VulkanCore::VCommandBuffer&               cmdBuffer,
                           const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                           VulkanUtils::RenderContext*               renderContext
)
{

    m_renderContextPtr = renderContext;
    //=====================================================
    // RECORD COMMAND BUFFER
    //=====================================================
    assert(cmdBuffer.GetIsRecording() && "Command buffer is not in recording state !");

    if(GlobalVariables::RenderingOptions::PreformDepthPrePass)
    {
        DepthPrePass(currentFrameIndex,cmdBuffer, uniformBufferManager);
    }
    //  DrawScene(currentFrameIndex,cmdBuffer, uniformBufferManager);


    m_frameCount++;
}

void SceneRenderer::CreateRenderTargets(VulkanCore::VSwapChain* swapChain)
{
    m_renderTargets = std::make_unique<Renderer::RenderTarget>(m_device, m_width, m_height);
}

void SceneRenderer::DrawScene(int currentFrameIndex,VulkanCore::VCommandBuffer& cmdBuffer, const VulkanUtils::VUniformBufferManager& uniformBufferManager)
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
    renderingInfo.pDepthAttachment     = &m_renderTargets->GetDepthAttachment();

    m_renderTargets->GetDepthAttachment().loadOp = vk::AttachmentLoadOp::eLoad;
    renderingInfo.pStencilAttachment             = &m_renderTargets->GetDepthAttachment();

    //==============================================
    // START RENDER PASS
    //==============================================
    auto& cmdB = cmdBuffer.GetCommandBuffer();

    VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets->GetColourImage(currentFrameIndex),
                                                    vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                    cmdBuffer);

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
    for(auto& drawCall : m_renderContextPtr->drawCalls)
    {
        auto& material = drawCall.second.material;
        if(drawCall.second.effect != currentEffect)
        {
            drawCall.second.effect->BindPipeline(cmdB);
            currentEffect = drawCall.second.effect;
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

        PushDataToGPU(cmdB, currentFrameIndex, drawCall.second.drawCallID, drawCall.second, uniformBufferManager);

        cmdB.drawIndexed(drawCall.second.indexData->size / sizeof(uint32_t), 1,
                              drawCall.second.indexData->offset / static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                              drawCall.second.vertexData->offset / static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)), 0);

        drawCallCount++;
    }

    cmdB.endRendering();

    VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets->GetColourImage(currentFrameIndex),
                                                    vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
                                                    cmdBuffer);

    m_renderingStatistics.DrawCallCount = drawCallCount;
}

void SceneRenderer::Destroy()
{
    m_renderTargets->Destroy();
    m_depthPrePassEffect->Destroy();
}
}  // namespace Renderer
