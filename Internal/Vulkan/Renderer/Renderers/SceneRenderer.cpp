//
// Created by wpsimon09 on 21/12/24.
//

#include "SceneRenderer.hpp"

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
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Pipeline/VPipelineManager.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"


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

        //---------------------------------------------------------------------------------------------------------------------------
        // CREATING TEMPLATE ENTRIES
        //---------------------------------------------------------------------------------------------------------------------------
        // global
        m_pushDescriptorManager.AddUpdateEntry(0, offsetof(VulkanUtils::DescriptorSetData, cameraUBOBuffer), 0);
        // per object
        m_pushDescriptorManager.AddUpdateEntry(1, offsetof(VulkanUtils::DescriptorSetData, meshUBBOBuffer), 0);
        // per material
        m_pushDescriptorManager.AddUpdateEntry(2, offsetof(VulkanUtils::DescriptorSetData, pbrMaterialNoTexture), 0);
        m_pushDescriptorManager.AddUpdateEntry(3, offsetof(VulkanUtils::DescriptorSetData, pbrMaterialFeatures), 0);
        m_pushDescriptorManager.AddUpdateEntry(4, offsetof(VulkanUtils::DescriptorSetData, diffuseTextureImage), 0);
        m_pushDescriptorManager.AddUpdateEntry(5, offsetof(VulkanUtils::DescriptorSetData, normalTextureImage), 0);
        m_pushDescriptorManager.AddUpdateEntry(6, offsetof(VulkanUtils::DescriptorSetData, armTextureImage), 0);
        m_pushDescriptorManager.AddUpdateEntry(7, offsetof(VulkanUtils::DescriptorSetData, emissiveTextureImage), 0);
        m_pushDescriptorManager.AddUpdateEntry(8, offsetof(VulkanUtils::DescriptorSetData, lightInformation), 0);
        m_pushDescriptorManager.AddUpdateEntry(9, offsetof(VulkanUtils::DescriptorSetData, LUT_LTC), 0);
        m_pushDescriptorManager.AddUpdateEntry(10, offsetof(VulkanUtils::DescriptorSetData, LUT_LTC_Inverse), 0);

        Utils::Logger::LogSuccess("Scene renderer created !");
    }

    void SceneRenderer::SendGlobalDescriptorsToShader(int currentFrameIndex,const VulkanUtils::VUniformBufferManager& uniformBufferManager)
    {
        auto& dstSetDataStruct = m_pushDescriptorManager.GetDescriptorSetDataStruct();
        dstSetDataStruct.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];
        dstSetDataStruct.lightInformation = uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrameIndex];
        dstSetDataStruct.LUT_LTC = MathUtils::LUT.LTC->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
        dstSetDataStruct.LUT_LTC_Inverse = MathUtils::LUT.LTCInverse->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
    }

    void SceneRenderer::SendPerObjectDescriptorsToShader(int currentFrameIndex,int objectIndex,VulkanStructs::DrawCallData& drawCall, const VulkanUtils::VUniformBufferManager& uniformBufferManager)
    {
        auto& dstSetDataStruct = m_pushDescriptorManager.GetDescriptorSetDataStruct();
        auto& material = drawCall.material;

        dstSetDataStruct.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(objectIndex)[
            currentFrameIndex];

        dstSetDataStruct.diffuseTextureImage =
            material->GetTexture(Diffues)->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

        dstSetDataStruct.armTextureImage =
            material->GetTexture(arm)->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

        dstSetDataStruct.normalTextureImage =
            material->GetTexture(normal)->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

        dstSetDataStruct.emissiveTextureImage =
            material->GetTexture(emissive)->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

        dstSetDataStruct.pbrMaterialFeatures = uniformBufferManager.GetMaterialFeaturesDescriptorBufferInfo(objectIndex)[
            currentFrameIndex];

        dstSetDataStruct.pbrMaterialNoTexture = uniformBufferManager.GetPerMaterialNoMaterialDescrptorBufferInfo(objectIndex)[
            currentFrameIndex];

    }


    void SceneRenderer::Init(const VulkanCore::VPipelineManager* pipelineManager)
    {
        m_pipelineManager = pipelineManager;
    }


    void SceneRenderer::Render(int currentFrameIndex,
                               const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                               VulkanStructs::RenderContext* renderContext
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
            pipelineType = EPipelineType::RasterPBRTextured;
        }

        if (m_multiLightShader)
        {
            pipelineType = EPipelineType::MultiLight;
        }


        RecordCommandBuffer(currentFrameIndex, uniformBufferManager,m_pipelineManager->GetPipeline(pipelineType));

        m_commandBuffers[currentFrameIndex]->EndRecording();

        //=====================================================
        // SUBMIT RECORDED COMMAND BUFFER
        //=====================================================
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[currentFrameIndex]->GetCommandBuffer();

        std::vector<vk::Semaphore> signalSemahores = {
            m_rendererFinishedSemaphore[currentFrameIndex]->GetSyncPrimitive()
        };
        submitInfo.signalSemaphoreCount = signalSemahores.size();
        submitInfo.pSignalSemaphores = signalSemahores.data();


        assert(m_device.GetGraphicsQueue().submit(1, &submitInfo, nullptr) == vk::Result::eSuccess &&
            "Failed to submit command buffer !");

    }

    void SceneRenderer::CreateRenderTargets(VulkanCore::VSwapChain* swapChain)
    {
        m_renderTargets = std::make_unique<Renderer::RenderTarget>(m_device, m_width, m_height);
    }

    void SceneRenderer::RecordCommandBuffer(int currentFrameIndex,
                                            const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                            const VulkanCore::VGraphicsPipeline& pipeline)
    {

        int  drawCallCount = 0;
        //==============================================
        // CREATE RENDER PASS INFO
        //==============================================
        vk::RenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.renderPass = GetRenderPass(currentFrameIndex).GetRenderPass();
        renderPassBeginInfo.framebuffer = GetFrameBuffer(currentFrameIndex).GetFrameBuffer();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>(GetTargeWidth()),
        renderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(GetTargeHeight());

        //==============================================
        // CONFIGURE CLEAR
        //==============================================
        std::array<vk::ClearValue, 2> clearColors = {};
        clearColors[0].color = {0.2f, 0.2f, 0.2f, 1.0f};
        clearColors[1].depthStencil.depth = 1.0f;
        clearColors[1].depthStencil.stencil = 0.0f;
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
        renderPassBeginInfo.pClearValues = clearColors.data();

        //==============================================
        // START RENDER PASS
        //==============================================
        auto& cmdBuffer = m_commandBuffers[currentFrameIndex]->GetCommandBuffer();

        cmdBuffer.beginRenderPass(
            &renderPassBeginInfo, vk::SubpassContents::eInline);

        cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineInstance());


        //===============================================
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

        if(m_renderContextPtr->MainLightPassOpaque.empty()){
            cmdBuffer.endRenderPass();
            m_renderingStatistics.DrawCallCount = drawCallCount;
            m_selectedGeometryDrawCalls.clear();
            return;
        }
        //=================================================
        // UPDATE DESCRIPTOR SETS
        //=================================================
        SendGlobalDescriptorsToShader(currentFrameIndex, uniformBufferManager);

        auto initialVertexBuffer = m_renderContextPtr->MainLightPassOpaque[0].meshData->vertexData.buffer;  
        auto initialIndexBuffer = m_renderContextPtr->MainLightPassOpaque[0].meshData->indexData.buffer;  
    
        cmdBuffer.bindVertexBuffers(0, {initialVertexBuffer}, {0});
        cmdBuffer.bindIndexBuffer(initialIndexBuffer, 0, vk::IndexType::eUint32);
    
        //=================================================
        // RECORD OPAQUE DRAW CALLS
        //=================================================    

        for (int i = 0; i < m_renderContextPtr->MainLightPassOpaque.size(); i++)
        {
            
            auto& drawCall = m_renderContextPtr->MainLightPassOpaque[i];
            auto& material = drawCall.material;
            SendPerObjectDescriptorsToShader(currentFrameIndex, i, drawCall, uniformBufferManager);
            
            //================================================================================================
            // BIND VERTEX BUFFER ONLY IF IT HAS CHANGED
            //================================================================================================        
            
            if(initialVertexBuffer != drawCall.meshData->vertexData.buffer){
                auto firstBinding = 0;
                std::vector<vk::Buffer> vertexBuffers = {initialVertexBuffer};
                std::vector<vk::DeviceSize> offsets = {drawCall.meshData->vertexData.offset};
                vertexBuffers = {drawCall.meshData->vertexData.buffer};
                cmdBuffer.bindVertexBuffers(firstBinding, vertexBuffers, offsets);
            }

            if(initialIndexBuffer != drawCall.meshData->indexData.buffer){
                cmdBuffer.bindIndexBuffer(drawCall.meshData->indexData.buffer, 0, vk::IndexType::eUint32);
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


        
        //=================================================
        // RECORD TRANSPARENT DRAW CALLS
        //================================================= 
        // TODO: maybe send global stuff to shaders, maybe not 
        
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
            
            if(initialVertexBuffer != drawCall.meshData->vertexData.buffer){
                auto firstBinding = 0;
                std::vector<vk::Buffer> vertexBuffers = {initialVertexBuffer};
                std::vector<vk::DeviceSize> offsets = {drawCall.meshData->vertexData.offset};
                vertexBuffers = {drawCall.meshData->vertexData.buffer};
                cmdBuffer.bindVertexBuffers(firstBinding, vertexBuffers, offsets);
            }

            if(initialIndexBuffer != drawCall.meshData->indexData.buffer){
                cmdBuffer.bindIndexBuffer(drawCall.meshData->indexData.buffer, 0, vk::IndexType::eUint32);
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

        cmdBuffer.endRenderPass();

        m_renderingStatistics.DrawCallCount = drawCallCount;
        m_selectedGeometryDrawCalls.clear();
    }

    void SceneRenderer::Destroy()
    {
        BaseRenderer::Destroy();
        m_sceneCommandPool->Destroy();
    }
} // Renderer
