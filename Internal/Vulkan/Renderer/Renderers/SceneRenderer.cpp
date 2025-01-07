//
// Created by wpsimon09 on 21/12/24.
//

#include "SceneRenderer.hpp"

#include "DebugRenderer.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
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
                                              m_device(device), m_selectedGeometry{}

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


        Utils::Logger::LogSuccess("Scene renderer created !");
    }

    void SceneRenderer::Init(const VulkanCore::VPipelineManager* pipelineManager)
    {
        m_pipelineManager = pipelineManager;
    }


    void SceneRenderer::Render(int currentFrameIndex,
                               const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                               const VulkanStructs::RenderContext& renderContext
    )
    {
        m_renderContextPtr = &renderContext;

        auto& renderTarget = m_renderTargets;
        m_commandBuffers[currentFrameIndex]->Reset();

        //=====================================================
        // RECORD COMMAND BUFFER
        //=====================================================ň
        m_commandBuffers[currentFrameIndex]->BeginRecording();

        EPipelineType pipelineType;
        if (m_WireFrame)
        {
            pipelineType = EPipelineType::DebugLines;
        }else if (m_renderContextPtr->metaData.bRasterPass)
        {
            pipelineType = EPipelineType::RasterPBRTextured;
        }else if (m_renderContextPtr->metaData.bRTXPass)
        {
            pipelineType = EPipelineType::RTX;
        }

        RecordCommandBuffer(currentFrameIndex, uniformBufferManager,m_pipelineManager->GetPipeline(pipelineType));

        m_commandBuffers[currentFrameIndex]->EndRecording();
        m_selectedGeometry.DrawCalls.clear();

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

        //=================================================
        // UPDATE DESCRIPTOR SETS
        //=================================================
        auto& dstSetDataStruct = m_pushDescriptorManager.GetDescriptorSetDataStruct();
        dstSetDataStruct.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];

        for (int i = 0; i < m_renderContextPtr->DrawCalls.size(); i++)
        {
            dstSetDataStruct.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(i)[
                currentFrameIndex];

            auto& drawCall = m_renderContextPtr->DrawCalls[i];
            auto& material = drawCall.material;

            dstSetDataStruct.diffuseTextureImage =
                material->GetTexture(PBR_DIFFUSE_MAP)->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

            dstSetDataStruct.armTextureImage =
                material->GetTexture(PBR_ARM)->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

            dstSetDataStruct.normalTextureImage =
                material->GetTexture(PBR_NORMAL_MAP)->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

            dstSetDataStruct.emissiveTextureImage =
                material->GetTexture(PBR_EMISSIVE_MAP)->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

            dstSetDataStruct.pbrMaterialFeatures = uniformBufferManager.GetMaterialFeaturesDescriptorBufferInfo(i)[
                currentFrameIndex];

            dstSetDataStruct.pbrMaterialNoTexture = uniformBufferManager.GetPerMaterialNoMaterialDescrptorBufferInfo(i)[
                currentFrameIndex];

            std::vector<vk::Buffer> vertexBuffers = {m_renderContextPtr->DrawCalls[i].vertexBuffer};
            std::vector<vk::DeviceSize> offsets = {0};

            cmdBuffer.bindIndexBuffer(drawCall.indexBuffer, 0, vk::IndexType::eUint32);
            cmdBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

            cmdBuffer.pushDescriptorSetWithTemplateKHR(
                m_pushDescriptorManager.GetTemplate(),
                pipeline.GetPipelineLayout(), 0,
                dstSetDataStruct, m_device.DispatchLoader);

            cmdBuffer.drawIndexed(drawCall.indexCount, 1, 0, 0, 0);
            drawCallCount++;

            if (drawCall.renderOutline)
            {
                m_selectedGeometry.DrawCalls.emplace_back(drawCall);
            }

        }

        if (m_AllowDebugDraw)
        {
            drawCallCount += RecordCommandBufferToDrawDebugGeometry(m_device, currentFrameIndex, cmdBuffer, uniformBufferManager,
                                                   m_pushDescriptorManager, *m_renderContextPtr,
                                                   m_pipelineManager->GetPipeline(EPipelineType::DebugLines));
        }

        // renders the outline
        drawCallCount += DrawSelectedMeshes(m_device, currentFrameIndex, cmdBuffer, uniformBufferManager,
                                                   m_pushDescriptorManager, m_selectedGeometry,
                                                    m_pipelineManager->GetPipeline(EPipelineType::Outline));

        cmdBuffer.endRenderPass();

        m_renderingStatistics.DrawCallCunt = drawCallCount;
    }

    void SceneRenderer::Destroy()
    {
        BaseRenderer::Destroy();
        m_sceneCommandPool->Destroy();
    }
} // Renderer
