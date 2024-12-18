//
// Created by wpsimon09 on 15/10/24.
//

#include "VRenderer.hpp"
#include "VRenderer.hpp"

#include <thread>
#include <vulkan/vulkan_handles.hpp>

#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Application/Client.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/VulkanCore/Pipeline/VPipelineManager.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore//VImage/VImage.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"


namespace Renderer
{

    VRenderer:: VRenderer(const VulkanCore::VulkanInstance &instance, const VulkanCore::VDevice &device,
                        const VulkanUtils::VUniformBufferManager &uniformBufferManager,
                         VulkanUtils::VPushDescriptorManager &pushDescriptorSetManager):
        m_device(device), m_uniformBufferManager(uniformBufferManager),
        m_pushDescriptorSetManager(pushDescriptorSetManager), m_rasterRenderContext(), m_rayTracingRenderContext() {

        m_depthBuffer = std::make_unique<VulkanCore::VImage>(m_device, 1, m_device.GetDepthFormat(), vk::ImageAspectFlagBits::eDepth);
        m_swapChain = std::make_unique<VulkanCore::VSwapChain>(device, instance, *m_depthBuffer);
        m_mainRenderPass = std::make_unique<VulkanCore::VRenderPass>(device, *m_swapChain, *m_depthBuffer);
        m_pipelineManager = std::make_unique<VulkanCore::VPipelineManager>(
            device, *m_swapChain, *m_mainRenderPass, m_pushDescriptorSetManager);
        m_pipelineManager->InstantiatePipelines();
        m_swapChain->CreateSwapChainFrameBuffers(*m_mainRenderPass);
        m_graphicsPipeline = &m_pipelineManager->GetPipeline(PIPELINE_TYPE_RASTER_PBR_TEXTURED);
        CreateCommandBufferPools();
        CreateSyncPrimitives();
        CreateTemplateEntries();
        m_pushDescriptorSetManager.CreateUpdateTemplate(*m_graphicsPipeline);


        m_rayTracingRenderContext.metaData.bRasterPass = false;
        m_rayTracingRenderContext.metaData.bRTXPass = true;

        m_renderingContext = &m_rasterRenderContext;
    }

    void VRenderer::Render(GlobalUniform& globalUniformUpdateInfo) {
        vk::Pipeline pipeline;
        if (m_isRTXOn)
        {
            pipeline = m_pipelineManager->GetPipeline(PIPELINE_TYPE_RTX).GetPipelineInstance();
            m_renderingContext = &m_rayTracingRenderContext;
        }else
        {
            pipeline = m_pipelineManager->GetPipeline(PIPELINE_TYPE_RASTER_PBR_TEXTURED).GetPipelineInstance();
            m_renderingContext = &m_rasterRenderContext;
        }
        m_isFrameFinishFences[m_currentFrameIndex]->WaitForFence();
        //rerender the frame if image to present on is out of date
        if (FetchSwapChainImage() == vk::Result::eEventReset) {
            Utils::Logger::LogInfoVerboseRendering("Received event reset signal, resetting the rendering....");
            return;
        }
        m_isFrameFinishFences[m_currentFrameIndex]->ResetFence();
        m_baseCommandBuffers[m_currentFrameIndex]->Reset();

        m_uniformBufferManager.UpdatePerFrameUniformData(m_currentFrameIndex,globalUniformUpdateInfo);
        m_uniformBufferManager.UpdatePerObjectUniformData(m_currentFrameIndex, m_renderingContext->DrawCalls);

        m_baseCommandBuffers[m_currentFrameIndex]->BeginRecording();
        StartRenderPass();
        //RecordCommandBuffersForPipelines(m_graphicsPipeline->GetPipelineInstance());

        RecordCommandBuffersForPipelines(pipeline);
        EndRenderPass();
        m_baseCommandBuffers[m_currentFrameIndex]->EndRecording();
        std::lock_guard<std::mutex> lock(m_device.DeviceMutex);
        SubmitCommandBuffer();
        PresentResults();
        m_renderingContext->DrawCalls.clear();
        m_currentFrameIndex = (m_currentImageIndex + 1) % GlobalVariables::MAX_FRAMES_IN_FLIGHT;
    }

    void VRenderer::CreateCommandBufferPools() {

        Utils::Logger::LogInfo("Allocating command pools...");
        m_baseCommandPool = std::make_unique<VulkanCore::VCommandPool>(m_device, QUEUE_FAMILY_INDEX_GRAPHICS);
        m_baseCommandBuffers.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
            m_baseCommandBuffers[i] = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_baseCommandPool);
        }
        Utils::Logger::LogInfo("Command pools and command buffers allocated !");
    }

    //==============================================================================
    // FOR COMMAND BUFFER
    //==============================================================================
    void VRenderer::StartRenderPass() {
        vk::RenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.renderPass = m_mainRenderPass->GetRenderPass();
        renderPassBeginInfo.framebuffer = m_swapChain->GetSwapChainFrameBuffers()[m_currentImageIndex].get().
            GetFrameBuffer();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = m_swapChain->GetExtent();

        std::array<vk::ClearValue,2> clearColors = {};
        clearColors[0].color =  {0.2f, 0.2f, 0.2f, 1.0f};
        clearColors[1].depthStencil.depth = 1.0f;
        clearColors[1].depthStencil.stencil = 0.0f;

        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
        renderPassBeginInfo.pClearValues = clearColors.data();

        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().beginRenderPass(
            &renderPassBeginInfo, vk::SubpassContents::eInline);
    }

    void VRenderer::RecordCommandBuffersForPipelines(const vk::Pipeline &pipeline) {
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().bindPipeline(
            vk::PipelineBindPoint::eGraphics, pipeline);
        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;

        viewport.width = static_cast<float>(m_swapChain->GetExtent().width);
        viewport.height = static_cast<float>(m_swapChain->GetExtent().height);

        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().setViewport(0, 1, &viewport);

        vk::Rect2D scissors{};
        scissors.offset.x = 0;
        scissors.offset.y = 0;
        scissors.extent = m_swapChain->GetExtent();
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().setScissor(0, 1, &scissors);

        m_pushDescriptorSetManager.GetDescriptorSetDataStruct().cameraUBOBuffer = m_uniformBufferManager.
            GetGlobalBufferDescriptorInfo()[m_currentFrameIndex];

        // for each mesh
        for (int i = 0; i < m_renderingContext->DrawCalls.size(); i++)
        {

            m_pushDescriptorSetManager.GetDescriptorSetDataStruct().meshUBBOBuffer = m_uniformBufferManager.
                GetPerObjectDescriptorBufferInfo(i)[m_currentFrameIndex];

            m_pushDescriptorSetManager.GetDescriptorSetDataStruct().diffuseTextureImage =
            m_renderingContext->DrawCalls[i].material->GetTexture(PBR_DIFFUSE_MAP)->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

            m_pushDescriptorSetManager.GetDescriptorSetDataStruct().armTextureImage =
            m_renderingContext->DrawCalls[i].material->GetTexture(PBR_ARM)->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

            m_pushDescriptorSetManager.GetDescriptorSetDataStruct().normalTextureImage =
            m_renderingContext->DrawCalls[i].material->GetTexture(PBR_NORMAL_MAP)->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

            m_pushDescriptorSetManager.GetDescriptorSetDataStruct().emissiveTextureImage =
            m_renderingContext->DrawCalls[i].material->GetTexture(PBR_EMISSIVE_MAP)->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

            m_pushDescriptorSetManager.GetDescriptorSetDataStruct().pbrMaterialFeatures =
            m_uniformBufferManager.GetMaterialFeaturesDescriptorBufferInfo(i)[m_currentFrameIndex];

            m_pushDescriptorSetManager.GetDescriptorSetDataStruct().pbrMaterialNoTexture =
            m_uniformBufferManager.GetPerMaterialNoMaterialDescrptorBufferInfo(i)[m_currentFrameIndex];


            std::vector<vk::Buffer> vertexBuffers = {m_renderingContext->DrawCalls[i].vertexBuffer};
            std::vector<vk::DeviceSize> offsets = {0};

            m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().bindIndexBuffer(
                m_renderingContext->DrawCalls[i].indexBuffer,
                0,
                vk::IndexType::eUint32);

            m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().bindVertexBuffers(
                0, 1,
                vertexBuffers.data(),
                offsets.data());

                PushDescriptors();

                m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().drawIndexed(
                    m_renderingContext->DrawCalls[i].indexCount, 1, 0, 0, 0);
        }
    }

    void VRenderer::PushDescriptors() {
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().pushDescriptorSetWithTemplateKHR(
            m_pushDescriptorSetManager.GetTemplate(),
            m_graphicsPipeline->GetPipelineLayout(), 0,
            m_pushDescriptorSetManager.GetDescriptorSetDataStruct(),
            m_device.DispatchLoader);
    }

    void VRenderer::EndRenderPass() {
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().endRenderPass();
    }

    void VRenderer::CreateSyncPrimitives() {
        m_imageAvailableSemaphores.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_isFrameFinishFences.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
            m_imageAvailableSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
            m_renderFinishedSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
            m_isFrameFinishFences[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device, true);
        }
    }

    void VRenderer::CreateTemplateEntries() {
        // global
        m_pushDescriptorSetManager.AddUpdateEntry(0, offsetof(VulkanUtils::DescriptorSetData, cameraUBOBuffer), 0);
        // per object
        m_pushDescriptorSetManager.AddUpdateEntry(1, offsetof(VulkanUtils::DescriptorSetData, meshUBBOBuffer), 0);
        // per material
        m_pushDescriptorSetManager.AddUpdateEntry(2, offsetof(VulkanUtils::DescriptorSetData, pbrMaterialNoTexture), 0);
        m_pushDescriptorSetManager.AddUpdateEntry(3, offsetof(VulkanUtils::DescriptorSetData, pbrMaterialFeatures), 0);
        m_pushDescriptorSetManager.AddUpdateEntry(4, offsetof(VulkanUtils::DescriptorSetData, diffuseTextureImage), 0);
        m_pushDescriptorSetManager.AddUpdateEntry(5, offsetof(VulkanUtils::DescriptorSetData, normalTextureImage), 0);
        m_pushDescriptorSetManager.AddUpdateEntry(6, offsetof(VulkanUtils::DescriptorSetData, armTextureImage), 0);
        m_pushDescriptorSetManager.AddUpdateEntry(7, offsetof(VulkanUtils::DescriptorSetData, emissiveTextureImage), 0);
    }

    //===============================================================================================================


    //===============================================================================================================
    // FOR RENDER DRAWING
    //=================================================================================================================

    vk::Result VRenderer::FetchSwapChainImage() {

        auto imageIndex = VulkanUtils::SwapChainNextImageKHRWrapper(m_device, *m_swapChain, UINT64_MAX,
                                                                    *m_imageAvailableSemaphores[m_currentFrameIndex],
                                                                    nullptr);
        switch (imageIndex.first) {
        case vk::Result::eSuccess: {
            m_currentImageIndex = imageIndex.second;
            Utils::Logger::LogInfoVerboseRendering("Swap chain is successfuly retrieved");
            return vk::Result::eSuccess;
        }
        case vk::Result::eErrorOutOfDateKHR: {
            m_swapChain->RecreateSwapChain(*m_mainRenderPass);
            Utils::Logger::LogError("Swap chain was out of date, trying to recreate it...  ");
            return vk::Result::eEventReset;
        }
        case vk::Result::eSuboptimalKHR: {
            throw std::runtime_error("Suboptimal swap chain retrieved");
        };
        default:
            break;
        }
    }

    void VRenderer::SubmitCommandBuffer() {
        assert(!m_baseCommandBuffers[m_currentFrameIndex]->GetIsRecording());
        vk::SubmitInfo submitInfo;
        std::array<vk::Semaphore, 1> waitSemaphores = {
            m_imageAvailableSemaphores[m_currentFrameIndex]->GetSyncPrimitive()};
        std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer();

        std::vector<vk::Semaphore> signalSemaphores = {
            m_renderFinishedSemaphores[m_currentFrameIndex]->GetSyncPrimitive()};
        submitInfo.signalSemaphoreCount = signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();

        assert(
            m_device.GetGraphicsQueue().submit( 1 ,&submitInfo, m_isFrameFinishFences[m_currentFrameIndex]->
                GetSyncPrimitive()) == vk::Result::eSuccess);
        Utils::Logger::LogInfoVerboseRendering("Successfully submitted the command buffer");
    }

    void VRenderer::PresentResults() {
        vk::PresentInfoKHR presentInfo;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrameIndex]->GetSyncPrimitive();

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain->GetSwapChain();
        presentInfo.pImageIndices = &m_currentImageIndex;
        presentInfo.pResults = nullptr;
        vk::Result result = VulkanUtils::PresentQueueWrapper(m_device.GetPresentQueue(), presentInfo);
        if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR) {
            m_swapChain->RecreateSwapChain(*m_mainRenderPass);
        }
        Utils::Logger::LogInfoVerboseRendering("Image presented to the view successfully");
    }

    void VRenderer::Destroy() {
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
            m_imageAvailableSemaphores[i]->Destroy();
            m_renderFinishedSemaphores[i]->Destroy();
            m_isFrameFinishFences[i]->Destroy();
            m_baseCommandBuffers[i]->Destroy();
        }
        m_mainRenderPass->Destroy();
        m_pipelineManager->DestroyPipelines();
        m_baseCommandPool->Destroy();
        m_swapChain->Destroy();
        m_depthBuffer->Destroy();
    }

} // Renderer
