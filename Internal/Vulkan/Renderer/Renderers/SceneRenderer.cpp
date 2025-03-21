//
// Created by wpsimon09 on 21/12/24.
//

#include "SceneRenderer.hpp"

#include <sys/wait.h>
#include <Vulkan/Utils/VIimageTransitionCommands.hpp>

#include "DebugRenderer.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Utils/LinearyTransformedCosinesValues.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VDescriptorSetStructs.hpp"


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

        Utils::Logger::LogSuccess("Scene renderer created !");
    }


    void SceneRenderer::PushDataToGPU(const vk::CommandBuffer& cmdBuffer, int currentFrameIndex,int objectIndex,VulkanStructs::DrawCallData& drawCall, const VulkanUtils::VUniformBufferManager& uniformBufferManager)
    {
        std::visit([this,&currentFrameIndex,&objectIndex,&drawCall, &uniformBufferManager, cmdBuffer ](auto& effectDstStruct) {
            auto& material = drawCall.material;
            using T = std::decay_t<decltype(effectDstStruct)>;

            if constexpr (std::is_same_v<T, VulkanUtils::BasicDescriptorSet>)
            {
                auto& basicEffect = static_cast<VulkanUtils::BasicDescriptorSet&>(effectDstStruct);
                basicEffect.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];
                basicEffect.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.drawCallID)[currentFrameIndex];


                cmdBuffer.pushDescriptorSetWithTemplateKHR(
                    drawCall.material->GetEffect()->GetUpdateTemplate(),
                    drawCall.material->GetEffect()->GetPipelineLayout(), 0,
                    basicEffect, m_device.DispatchLoader);


            }
            else if constexpr (std::is_same_v<T, VulkanUtils::UnlitSingleTexture>)
            {
                auto& unlitSingelTextureEffect = static_cast<VulkanUtils::UnlitSingleTexture&>(effectDstStruct);
                unlitSingelTextureEffect.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];
                unlitSingelTextureEffect.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.drawCallID)[currentFrameIndex];
                unlitSingelTextureEffect.texture = drawCall.material->GetTexture(ETextureType::Diffues)->GetHandle()->
                                                            GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

                cmdBuffer.pushDescriptorSetWithTemplateKHR(
                    drawCall.material->GetEffect()->GetUpdateTemplate(),
                    drawCall.material->GetEffect()->GetPipelineLayout(), 0,
                    unlitSingelTextureEffect, m_device.DispatchLoader);

            }
            else if constexpr (std::is_same_v<T, VulkanUtils::ForwardShadingDstSet>)
            {
                auto& forwardShaddingEffect = static_cast<VulkanUtils::ForwardShadingDstSet&>(effectDstStruct);
                forwardShaddingEffect.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];
                forwardShaddingEffect.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCall.drawCallID)[currentFrameIndex];;
                forwardShaddingEffect.lightInformation = uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrameIndex];

                forwardShaddingEffect.diffuseTextureImage =
                    drawCall.material->GetTexture(Diffues)->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

                forwardShaddingEffect.armTextureImage =
                    drawCall.material->GetTexture(arm)->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

                forwardShaddingEffect.normalTextureImage =
                    drawCall.material->GetTexture(normal)->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

                forwardShaddingEffect.emissiveTextureImage =
                    drawCall.material->GetTexture(emissive)->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

                forwardShaddingEffect.pbrMaterialFeatures = uniformBufferManager.GetMaterialFeaturesDescriptorBufferInfo(objectIndex)[
                    currentFrameIndex];

                forwardShaddingEffect.pbrMaterialNoTexture = uniformBufferManager.GetPerMaterialNoMaterialDescrptorBufferInfo(objectIndex)[
                    currentFrameIndex];

                forwardShaddingEffect.LUT_LTC = MathUtils::LUT.LTC->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
                forwardShaddingEffect.LUT_LTC_Inverse = MathUtils::LUT.LTCInverse->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

                cmdBuffer.pushDescriptorSetWithTemplateKHR(
                    drawCall.material->GetEffect()->GetUpdateTemplate(),
                    drawCall.material->GetEffect()->GetPipelineLayout(), 0,
                    forwardShaddingEffect, m_device.DispatchLoader);
            }

        }, drawCall.material->GetEffect()->GetEffectUpdateStruct());

    }


    void SceneRenderer::Render(int currentFrameIndex,
                               const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                               VulkanStructs::RenderContext* renderContext,
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

        EPipelineType pipelineType;
        if (m_WireFrame)
        {
            pipelineType = EPipelineType::DebugLines;
        }else
        {
            pipelineType = EPipelineType::MultiLight;
        }


        RecordCommandBuffer(currentFrameIndex, uniformBufferManager);

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
        renderingTimeLine.CpuWaitIdle(2);

        m_commandBuffers[currentFrameIndex]->Reset();
        m_commandBuffers[currentFrameIndex]->BeginRecording();
        VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets->GetColourImage(currentFrameIndex), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *m_commandBuffers[currentFrameIndex]);

        std::vector<vk::PipelineStageFlags> waitStagesTransfer = {
            vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader};

        m_commandBuffers[currentFrameIndex]->EndAndFlush(m_device.GetTransferQueue(),renderingTimeLine.GetSemaphore(), renderingTimeLine.GetSemaphoreSubmitInfo(2, 4), waitStagesTransfer.data());

        transferSemapohre.Reset();
    }

    void SceneRenderer::CreateRenderTargets(VulkanCore::VSwapChain* swapChain)
    {
        m_renderTargets = std::make_unique<Renderer::RenderTarget>(m_device, m_width, m_height);
    }

    void SceneRenderer::RecordCommandBuffer(int currentFrameIndex,
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

        //==============================================
        // START RENDER PASS
        //==============================================
        auto& cmdBuffer = m_commandBuffers[currentFrameIndex]->GetCommandBuffer();
        VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTargets->GetColourImage(currentFrameIndex), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, *m_commandBuffers[currentFrameIndex]);

        cmdBuffer.beginRendering(&renderingInfo);


        // if there is nothing to render end the render process
        if(m_renderContextPtr->drawCalls.empty()){
            cmdBuffer.endRendering();
            m_renderingStatistics.DrawCallCount = drawCallCount;
            m_selectedGeometryDrawCalls.clear();
            return;
        }
        //=================================================
        // UPDATE DESCRIPTOR SETS
        //=================================================

        auto currentVertexBuffer = m_renderContextPtr->drawCalls.begin()->second.meshData->vertexData;
        auto currentIndexBuffer = m_renderContextPtr->drawCalls.begin()->second.meshData->indexData;
        vk::DeviceSize indexBufferOffset = 0;

        cmdBuffer.bindVertexBuffers(0, {currentVertexBuffer.buffer}, {0});
        cmdBuffer.bindIndexBuffer(currentIndexBuffer.buffer, 0, vk::IndexType::eUint32);

        //============================================
        //===
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
            auto& material = drawCall.second.material;

            drawCall.second.material->GetEffect()->BindPipeline(cmdBuffer);

            //================================================================================================
            // BIND VERTEX BUFFER ONLY IF IT HAS CHANGED
            //================================================================================================
            if(currentVertexBuffer != drawCall.second.meshData->vertexData){
                auto firstBinding = 0;

                indexBufferOffset = (currentVertexBuffer.offset + currentVertexBuffer.size)/ static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex));

                std::vector<vk::Buffer> vertexBuffers = {drawCall.second.meshData->vertexData.buffer};
                std::vector<vk::DeviceSize> offsets = {0};
                vertexBuffers = {drawCall.second.meshData->vertexData.buffer};
                cmdBuffer.bindVertexBuffers(firstBinding, vertexBuffers, offsets);
                currentVertexBuffer = drawCall.second.meshData->vertexData;
            }

            if(currentIndexBuffer != drawCall.second.meshData->indexData){
                indexBufferOffset = 0;
                cmdBuffer.bindIndexBuffer(drawCall.second.meshData->indexData.buffer, 0, vk::IndexType::eUint32);
                currentIndexBuffer = drawCall.second.meshData->indexData;
            }

            PushDataToGPU(cmdBuffer, currentFrameIndex, drawCall.second.drawCallID, drawCall.second, uniformBufferManager);

            cmdBuffer.drawIndexed(
                drawCall.second.meshData->indexData.size/sizeof(uint32_t),
                1,
                drawCall.second.meshData->indexData.offset/static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                    drawCall.second.meshData->vertexData.offset/static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)),
                0);

            drawCallCount++;

        }


        /**
        for (int i = 0; i < m_renderContextPtr->MainLightPassOpaque.size(); i++)
        {

            auto& drawCall = m_renderContextPtr->MainLightPassOpaque[i];
            auto& material = drawCall.material;
            SendPerObjectDescriptorsToShader(currentFrameIndex, i, drawCall, uniformBufferManager);

            //================================================================================================
            // BIND VERTEX BUFFER ONLY IF IT HAS CHANGED
            //================================================================================================
            if(currentVertexBuffer != drawCall.meshData->vertexData){
                auto firstBinding = 0;

                indexBufferOffset = (currentVertexBuffer.offset + currentVertexBuffer.size)/ static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex));

                std::vector<vk::Buffer> vertexBuffers = {drawCall.meshData->vertexData.buffer};
                std::vector<vk::DeviceSize> offsets = {0};
                vertexBuffers = {drawCall.meshData->vertexData.buffer};
                cmdBuffer.bindVertexBuffers(firstBinding, vertexBuffers, offsets);
                currentVertexBuffer = drawCall.meshData->vertexData;
            }

            if(currentIndexBuffer != drawCall.meshData->indexData){
                indexBufferOffset = 0;
                cmdBuffer.bindIndexBuffer(drawCall.meshData->indexData.buffer, 0, vk::IndexType::eUint32);
                currentIndexBuffer = drawCall.meshData->indexData;
            }

            cmdBuffer.pushDescriptorSetWithTemplateKHR(
                m_pushDescriptorManager.GetTemplate(),
                drawCall.material->GetEffect()->GetPipelineLayout(), 0,
                m_pushDescriptorManager.GetDescriptorSetDataStruct(), m_device.DispatchLoader);

            cmdBuffer.drawIndexed(
                drawCall.meshData->indexData.size/sizeof(uint32_t),
                1,
                drawCall.meshData->indexData.offset/static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                    drawCall.meshData->vertexData.offset/static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)),
                0);

            drawCallCount++;

            if (drawCall.renderOutline)
            {
                m_selectedGeometryDrawCalls.emplace_back(drawCall);
            }
        }
        **/



        //=================================================
        // RECORD TRANSPARENT DRAW CALLS
        //=================================================
        /**
        if(!m_renderContextPtr->MainLightPassTransparent.empty()){
            cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelineManager->GetPipeline(EPipelineType::Transparent).GetPipelineInstance());
        }

        for(int i = 0; i < m_renderContextPtr->MainLightPassTransparent.size(); i++)
        {
            auto& drawCall = m_renderContextPtr->MainLightPassTransparent[i];
            auto& material = drawCall.material;
            SendPerObjectDescriptorsToShader(currentFrameIndex, drawCall.drawCallID, drawCall, uniformBufferManager);

            //================================================================================================
            // BIND VERTEX BUFFER ONLY IF IT HAS CHANGED
            //================================================================================================

            if(currentVertexBuffer != drawCall.meshData->vertexData){
                std::vector<vk::Buffer> vertexBuffers = {drawCall.meshData->vertexData.buffer};
                std::vector<vk::DeviceSize> offsets = {0};
                cmdBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
                currentVertexBuffer = drawCall.meshData->vertexData;
            }

            if(currentIndexBuffer != drawCall.meshData->indexData){
                cmdBuffer.bindIndexBuffer(drawCall.meshData->indexData.buffer, 0, vk::IndexType::eUint32);
                currentIndexBuffer = drawCall.meshData->indexData;
            }

            cmdBuffer.pushDescriptorSetWithTemplateKHR(
                m_pushDescriptorManager.GetTemplate(),
                pipeline.GetPipelineLayout(), 0,
                m_pushDescriptorManager.GetDescriptorSetDataStruct(), m_device.DispatchLoader);

            cmdBuffer.drawIndexed(
                drawCall.meshData->indexData.size/sizeof(uint32_t),
                1,
                drawCall.meshData->indexData.offset/static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                drawCall.meshData->vertexData.offset/static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)),
                0);

            drawCallCount++;

            if (drawCall.renderOutline)
            {
                m_selectedGeometryDrawCalls.emplace_back(drawCall);
            }
        }


        /**
        //=================================================
        // RECORD AABB DRAW CALLS
        //=================================================    
        if (m_AllowDebugDraw)
        {
            std::vector<VulkanStructs::DrawCallData> drawCalls;
            m_renderContextPtr->GetAllDrawCall(drawCalls);
    
            drawCallCount += RecordCommandBufferToDrawDebugGeometry(m_device, currentFrameIndex, cmdBuffer, uniformBufferManager,
                                                   m_pushDescriptorManager, drawCalls,
                                                   m_pipelineManager->GetPipeline(EPipelineType::DebugLines));
        }


        //=================================================
        // RECORD OPAQUE DRAW CALLS FOR SELECTED OBJECTS
        //=================================================    
        if (!m_renderContextPtr->SelectedGeometryPass.empty())
        {
            // renders the outline
            drawCallCount += DrawSelectedMeshes(m_device, currentFrameIndex, cmdBuffer, uniformBufferManager,
                                                       m_pushDescriptorManager, m_renderContextPtr->SelectedGeometryPass,
                                                        m_pipelineManager->GetPipeline(EPipelineType::Outline));
        }

        //=================================================
        // RECORD DEBUG GEOMETRY DRAW CALLS
        //=================================================    
        if(!m_renderContextPtr->DebugGeometryPass.empty()){
            drawCallCount += DrawSelectedMeshes(m_device, currentFrameIndex, cmdBuffer, uniformBufferManager,
                m_pushDescriptorManager, m_renderContextPtr->DebugGeometryPass,
                 m_pipelineManager->GetPipeline(EPipelineType::DebugShadpes));
        }

        //=================================================
        // RECORD BILLBOARDS DRAW CALLS 
        //=================================================    
        if (m_allowEditorBillboards) {
            // draws editor bilboards
            drawCallCount += DrawEditorBillboards(m_device, currentFrameIndex, cmdBuffer, uniformBufferManager,
                                                   m_pushDescriptorManager, m_renderContextPtr->EditorBillboardPass,
                                                    m_pipelineManager->GetPipeline(EPipelineType::EditorBillboard));
        }

        */
        cmdBuffer.endRendering();


        m_renderingStatistics.DrawCallCount = drawCallCount;
        m_selectedGeometryDrawCalls.clear();
    }

    void SceneRenderer::Destroy()
    {
        BaseRenderer::Destroy();
        m_sceneCommandPool->Destroy();
    }
} // Renderer
