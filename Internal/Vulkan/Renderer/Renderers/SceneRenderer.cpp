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
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VDescriptorSetStructs.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"


namespace Renderer
{
    SceneRenderer::SceneRenderer(const VulkanCore::VDevice& device,
                                 VulkanUtils::VPushDescriptorManager& pushDescriptorManager, int width,
                                 int height): BaseRenderer(device),
                                              m_pushDescriptorManager(pushDescriptorManager),
                                              m_device(device)

    {
        Utils::Logger::LogInfo("Creating scene renderer");

        m_width = width;
        m_height = height;
        SceneRenderer::CreateRenderTargets(nullptr);

        m_sceneCommandPool = std::make_unique<VulkanCore::VCommandPool>(device, Graphics);
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_commandBuffers[i] = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_sceneCommandPool);
        }

        //=========================
        // CONFIGURE DEAPTH PASS EFFECT
        //=========================
        m_depthPrePassEffect = std::make_unique<VulkanUtils::VEffect>(
            m_device, "Depth-PrePass effect",
            "Shaders/Compiled/DepthPrePass.vert.spv","Shaders/Compiled/DepthPrePass.frag.spv",
            m_pushDescriptorManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::Basic)  );
        m_depthPrePassEffect
            ->SetVertexInputMode(EVertexInput::PositionOnly)
            .SetDepthOpLess();

        m_depthPrePassEffect->BuildEffect();

        Utils::Logger::LogSuccess("Scene renderer created !");
    }


    void SceneRenderer::PushDataToGPU(const vk::CommandBuffer& cmdBuffer, int currentFrameIndex,int objectIndex,VulkanStructs::DrawCallData& drawCall, const VulkanUtils::VUniformBufferManager& uniformBufferManager)
    {
        switch (drawCall.effect->GetLayoutStructType())
        {
            case VulkanUtils::EDescriptorLayoutStruct::Basic:
                {
                    auto& basicEffecResourceGroup = std::get<VulkanUtils::BasicDescriptorSet>(drawCall.effect->GetEffectUpdateStruct());
                    basicEffecResourceGroup.buffer1 = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];
                    basicEffecResourceGroup.buffer2 = uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.drawCallID)[currentFrameIndex];

                    cmdBuffer.pushDescriptorSetWithTemplateKHR(
                        drawCall.effect->GetUpdateTemplate(),
                        drawCall.effect->GetPipelineLayout(), 0,
                        basicEffecResourceGroup, m_device.DispatchLoader);
                    break;
                }
            case VulkanUtils::EDescriptorLayoutStruct::UnlitSingleTexture:
                {
                    auto& unlitSingleTextureResrouceGroup = std::get<VulkanUtils::UnlitSingleTexture>(drawCall.effect->GetEffectUpdateStruct());;
                    unlitSingleTextureResrouceGroup.buffer1 = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];
                    unlitSingleTextureResrouceGroup.buffer2 = uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.drawCallID)[currentFrameIndex];

                    // cast might be required here
                    drawCall.material->UpdateGPUTextureData(unlitSingleTextureResrouceGroup);

                    if (drawCall.effect->GetName() == "Sky Box")
                    {
                        if (m_renderContextPtr->hdrCubeMap)
                        {
                            unlitSingleTextureResrouceGroup.texture2D_1 = m_renderContextPtr->hdrCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerClampToEdge);
                        }else
                            unlitSingleTextureResrouceGroup.texture2D_1 = m_renderContextPtr->dummyCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerClampToEdge);
                    }

                    cmdBuffer.pushDescriptorSetWithTemplateKHR(
                        drawCall.effect->GetUpdateTemplate(),
                        drawCall.effect->GetPipelineLayout(), 0,
                        unlitSingleTextureResrouceGroup, m_device.DispatchLoader);
                    break;
                }
            case VulkanUtils::EDescriptorLayoutStruct::ForwardShading:
                {
                    auto& forwardShadingResourceGroup = std::get<VulkanUtils::ForwardShadingDstSet>(drawCall.effect->GetEffectUpdateStruct());
                    forwardShadingResourceGroup.buffer1 = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];
                    forwardShadingResourceGroup.buffer2 = uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.drawCallID)[currentFrameIndex];;
                    forwardShadingResourceGroup.buffer3 = uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrameIndex];

                    /*
                    forwardShaddingEffect.buffer4 = uniformBufferManager.GetMaterialFeaturesDescriptorBufferInfo(objectIndex)[
                        currentFrameIndex];

                    forwardShaddingEffect.buffer5 = uniformBufferManager.GetPerMaterialNoMaterialDescrptorBufferInfo(objectIndex)[
                        currentFrameIndex];
                        */

                    drawCall.material->UpdateGPUTextureData(forwardShadingResourceGroup);

                    forwardShadingResourceGroup.texture2D_5 = MathUtils::LUT.LTC->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
                    forwardShadingResourceGroup.texture2D_6 = MathUtils::LUT.LTCInverse->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
                    if (m_renderContextPtr->irradianceMap){
                        forwardShadingResourceGroup.texture2D_7 = m_renderContextPtr->irradianceMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
                    }else{ forwardShadingResourceGroup.texture2D_7 = m_renderContextPtr->dummyCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);; }

                    if (m_renderContextPtr->prefilterMap){
                        forwardShadingResourceGroup.texture2D_8 = m_renderContextPtr->prefilterMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips);
                    }else{ forwardShadingResourceGroup.texture2D_8 = m_renderContextPtr->dummyCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);; }

                    if (m_renderContextPtr->brdfMap){
                        forwardShadingResourceGroup.texture2D_9 = m_renderContextPtr->brdfMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
                    }else{ forwardShadingResourceGroup.texture2D_9 = m_renderContextPtr->dummyCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);; }
                    //forwardShaddingEffect.texture2D_7 = m_renderContextPtr->irradianceMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

                    cmdBuffer.pushDescriptorSetWithTemplateKHR(
                        drawCall.effect->GetUpdateTemplate(),
                        drawCall.effect->GetPipelineLayout(), 0,
                        forwardShadingResourceGroup, m_device.DispatchLoader);
                    break;
                }
            }

    }

    void SceneRenderer::DepthPrePass(int currentFrameIndex,
        const VulkanUtils::VUniformBufferManager& uniformBufferManager)
    {
        int drawCallCount = 0;



        vk::RenderingInfo renderingInfo;
        renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderingInfo.renderArea.extent = vk::Extent2D(m_width, m_height);
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 0;
        renderingInfo.pColorAttachments = nullptr;
        m_renderTargets->GetDepthAttachment().loadOp = vk::AttachmentLoadOp::eClear;

        renderingInfo.pDepthAttachment = &m_renderTargets->GetDepthAttachment();

        m_depthPrePassEffect->BindPipeline(m_commandBuffers[currentFrameIndex]->GetCommandBuffer());


        //==============================================
        // START RENDER PASS
        //==============================================
        auto& cmdBuffer = m_commandBuffers[currentFrameIndex]->GetCommandBuffer();

        cmdBuffer.beginRendering(&renderingInfo);

        // if there is nothing to render end the render process
        if(m_renderContextPtr->drawCalls.empty()){
            cmdBuffer.endRendering();
            m_renderingStatistics.DrawCallCount = 0;
            return;
        }

        //=================================================
        // INITIAL CONFIG
        //=================================================
        auto currentVertexBuffer = m_renderContextPtr->drawCalls.begin()->second.vertexData;
        auto currentIndexBuffer = m_renderContextPtr->drawCalls.begin()->second.indexData;

        vk::DeviceSize indexBufferOffset = 0;

        cmdBuffer.bindVertexBuffers(0, {currentVertexBuffer->buffer}, {0});
        cmdBuffer.bindIndexBuffer(currentIndexBuffer->buffer, 0, vk::IndexType::eUint32);

        //============================================
        // CONFIGURE VIEW PORT
        //===============================================
        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;

        viewport.width = static_cast<float>(m_width);
        viewport.height = static_cast<float>(m_height);

        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        cmdBuffer.setViewport(0, 1, &viewport);

        vk::Rect2D scissors{};
        scissors.offset.x = 0;
        scissors.offset.y = 0;
        scissors.extent.width = m_width;
        scissors.extent.height = m_height;

        cmdBuffer.setScissor(0, 1, &scissors);

        //=================================================
        // RECORD OPAQUE DRAW CALLS
        //=================================================
        for (auto& drawCall: m_renderContextPtr->drawCalls)
        {

            if (drawCall.second.inDepthPrePass)
            {
                    cmdBuffer.setStencilTestEnable(false);

                    //================================================================================================
                    // BIND VERTEX BUFFER ONLY IF IT HAS CHANGED
                    //================================================================================================
                    if(currentVertexBuffer->BufferID != drawCall.second.vertexData->BufferID){
                        auto firstBinding = 0;

                        std::vector<vk::Buffer> vertexBuffers = {drawCall.second.vertexData->buffer};
                        std::vector<vk::DeviceSize> offsets = {0};
                        vertexBuffers = {drawCall.second.vertexData->buffer};
                       cmdBuffer.bindVertexBuffers(firstBinding, vertexBuffers, offsets);
                        currentVertexBuffer = drawCall.second.vertexData;
                    }

                    if(currentIndexBuffer->BufferID != drawCall.second.indexData->BufferID){
                        indexBufferOffset = 0;
                        cmdBuffer.bindIndexBuffer(drawCall.second.indexData->buffer, 0, vk::IndexType::eUint32);
                        currentIndexBuffer = drawCall.second.indexData;
                    }

                    auto& update = std::get<VulkanUtils::BasicDescriptorSet>(m_depthPrePassEffect->GetEffectUpdateStruct());
                    update.buffer1 = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];
                    update.buffer2 = uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.second.drawCallID)[currentFrameIndex];

                    cmdBuffer.pushDescriptorSetWithTemplateKHR(
                                    m_depthPrePassEffect->GetUpdateTemplate(),
                                    m_depthPrePassEffect->GetPipelineLayout(), 0,
                                    update, m_device.DispatchLoader);

                    cmdBuffer.drawIndexed(
                        drawCall.second.indexData->size/sizeof(uint32_t),
                        1,
                        drawCall.second.indexData->offset/  static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                            drawCall.second.vertexData->offset/static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)),
                        0);

                    drawCallCount++;

                }

            }
            cmdBuffer.endRendering();


            VulkanUtils::PlaceImageMemoryBarrier(
                m_renderTargets->GetDepthImage(currentFrameIndex), *m_commandBuffers[currentFrameIndex],
                vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::PipelineStageFlagBits::eEarlyFragmentTests,
                vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::AccessFlagBits::eDepthStencilAttachmentRead
                );

            m_renderingStatistics.DrawCallCount = drawCallCount;
    }


    void SceneRenderer::Render(int currentFrameIndex,
                               const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                               VulkanUtils::RenderContext* renderContext,
                               VulkanCore::VTimelineSemaphore& renderingTimeLine,
                               VulkanCore::VTimelineSemaphore& transferSemapohre

    )
    {

        m_renderContextPtr = renderContext;

        auto& renderTarget = m_renderTargets;
        m_commandBuffers[currentFrameIndex]->Reset();

        //=====================================================
        // RECORD COMMAND BUFFER
        //=====================================================
        m_commandBuffers[currentFrameIndex]->BeginRecording();

        if (m_depthPrePass)
        {
            DepthPrePass(currentFrameIndex, uniformBufferManager);
        }
        DrawScene(currentFrameIndex, uniformBufferManager);

        m_commandBuffers[currentFrameIndex]->EndRecording();

        //=====================================================
        // SUBMIT RECORDED COMMAND BUFFER
        //=====================================================
        vk::SubmitInfo submitInfo;

        const std::vector<vk::Semaphore> semaphores = {renderingTimeLine.GetSemaphore(), transferSemapohre.GetSemaphore()};

        std::vector<vk::PipelineStageFlags> waitStages = {
            vk::PipelineStageFlagBits::eColorAttachmentOutput, // Render wait stage
            vk::PipelineStageFlagBits::eTransfer               // Transfer wait stage
        };

        renderingTimeLine.SetWaitAndSignal(0, 2); //
        transferSemapohre.SetWaitAndSignal(2, 4);

        const std::vector<uint64_t> waitValues = {renderingTimeLine.GetCurrentWaitValue(), transferSemapohre.GetCurrentWaitValue()};
        const std::vector<uint64_t> signalVlaues = {renderingTimeLine.GetCurrentSignalValue(), transferSemapohre.GetCurrentSignalValue()};

        vk::TimelineSemaphoreSubmitInfo timelineinfo;
        timelineinfo.waitSemaphoreValueCount = waitValues.size();
        timelineinfo.pWaitSemaphoreValues = waitValues.data();

        timelineinfo.signalSemaphoreValueCount = signalVlaues.size();
        timelineinfo.pSignalSemaphoreValues = signalVlaues.data();

        submitInfo.pNext = &timelineinfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[currentFrameIndex]->GetCommandBuffer();

        submitInfo.signalSemaphoreCount = semaphores.size();
        submitInfo.pSignalSemaphores = semaphores.data();

        submitInfo.waitSemaphoreCount = semaphores.size();
        submitInfo.pWaitSemaphores =semaphores.data();

        submitInfo.pWaitDstStageMask = waitStages.data();

        assert(m_device.GetGraphicsQueue().submit(1, &submitInfo, nullptr) == vk::Result::eSuccess &&
            "Failed to submit command buffer !");

        transferSemapohre.Reset();
        m_frameCount++;
    }

    void SceneRenderer::CreateRenderTargets(VulkanCore::VSwapChain* swapChain)
    {
        m_renderTargets = std::make_unique<Renderer::RenderTarget>(m_device, m_width, m_height);
    }

    void SceneRenderer::DrawScene(int currentFrameIndex,
                                            const VulkanUtils::VUniformBufferManager& uniformBufferManager)
    {

        int  drawCallCount = 0;
        //==============================================
        // CREATE RENDER PASS INFO
        //==============================================
        std::vector<vk::RenderingAttachmentInfo> colourAttachments = {
            m_renderTargets->GetColourAttachmentMultiSampled(currentFrameIndex),
        };

        vk::RenderingInfo renderingInfo;
        renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderingInfo.renderArea.extent = vk::Extent2D(m_width, m_height);
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = colourAttachments.size();
        renderingInfo.pColorAttachments = colourAttachments.data();
        renderingInfo.pDepthAttachment = &m_renderTargets->GetDepthAttachment();

        m_renderTargets->GetDepthAttachment().loadOp = vk::AttachmentLoadOp::eLoad;
        renderingInfo.pStencilAttachment = &m_renderTargets->GetDepthAttachment();

        //==============================================
        // START RENDER PASS
        //==============================================
        auto& cmdBuffer = m_commandBuffers[currentFrameIndex]->GetCommandBuffer();
        if (m_frameCount > 0)
        {
            VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets->GetColourImage(currentFrameIndex), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *m_commandBuffers[currentFrameIndex]);
        }

        cmdBuffer.beginRendering(&renderingInfo);


        // if there is nothing to render end the render process
        if(m_renderContextPtr->drawCalls.empty()){
            cmdBuffer.endRendering();
            m_renderingStatistics.DrawCallCount = drawCallCount;
            return;
        }
        //=================================================
        // UPDATE DESCRIPTOR SETS
        //=================================================

        auto currentVertexBuffer = m_renderContextPtr->drawCalls.begin()->second.vertexData;
        auto currentIndexBuffer = m_renderContextPtr->drawCalls.begin()->second.indexData;
        auto& currentEffect = m_renderContextPtr->drawCalls.begin()->second.effect;

        vk::DeviceSize indexBufferOffset = 0;

        cmdBuffer.bindVertexBuffers(0, {currentVertexBuffer->buffer}, {0});
        cmdBuffer.bindIndexBuffer(currentIndexBuffer->buffer, 0, vk::IndexType::eUint32);

        //============================================
        // CONFIGURE VIEW PORT
        //===============================================
        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;

        viewport.width = static_cast<float>(m_width);
        viewport.height = static_cast<float>(m_height);

        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        cmdBuffer.setViewport(0, 1, &viewport);

        vk::Rect2D scissors{};
        scissors.offset.x = 0;
        scissors.offset.y = 0;
        scissors.extent.width = m_width;
        scissors.extent.height = m_height;


        cmdBuffer.setScissor(0, 1, &scissors);


        //=================================================
        // RECORD OPAQUE DRAW CALLS
        //=================================================
        currentEffect->BindPipeline(cmdBuffer);
        for (auto& drawCall: m_renderContextPtr->drawCalls)
        {
            auto& material = drawCall.second.material;
            if (drawCall.second.effect != currentEffect)
            {
                drawCall.second.effect->BindPipeline(cmdBuffer);
                currentEffect = drawCall.second.effect;
            }

            if (drawCall.second.selected) cmdBuffer.setStencilTestEnable(true);
            else                          cmdBuffer.setStencilTestEnable(false);
            //================================================================================================
            // BIND VERTEX BUFFER ONLY IF IT HAS CHANGED
            //================================================================================================
            if(currentVertexBuffer->BufferID != drawCall.second.vertexData->BufferID){
                auto firstBinding = 0;

                std::vector<vk::Buffer> vertexBuffers = {drawCall.second.vertexData->buffer};
                std::vector<vk::DeviceSize> offsets = {0};
                vertexBuffers = {drawCall.second.vertexData->buffer};
               cmdBuffer.bindVertexBuffers(firstBinding, vertexBuffers, offsets);
                currentVertexBuffer = drawCall.second.vertexData;
            }

            if(currentIndexBuffer->BufferID != drawCall.second.indexData->BufferID){
                indexBufferOffset = 0;
                cmdBuffer.bindIndexBuffer(drawCall.second.indexData->buffer, 0, vk::IndexType::eUint32);
                currentIndexBuffer = drawCall.second.indexData;
            }

            PushDataToGPU(cmdBuffer, currentFrameIndex, drawCall.second.drawCallID, drawCall.second, uniformBufferManager);

            cmdBuffer.drawIndexed(
                drawCall.second.indexData->size/sizeof(uint32_t),
                1,
                drawCall.second.indexData->offset/static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                    drawCall.second.vertexData->offset/static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)),
                0);

            drawCallCount++;

        }

        cmdBuffer.endRendering();

        VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets->GetColourImage(currentFrameIndex), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal, *m_commandBuffers[currentFrameIndex]);
        m_renderingStatistics.DrawCallCount = drawCallCount;
    }

    void SceneRenderer::Destroy()
    {
        BaseRenderer::Destroy();
        m_sceneCommandPool->Destroy();
        m_depthPrePassEffect->Destroy();
    }
} // Renderer
