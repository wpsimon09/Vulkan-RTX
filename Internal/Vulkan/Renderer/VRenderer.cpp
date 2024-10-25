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
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/VulkanCore/Pipeline/VPipelineManager.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/Utils/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"


namespace Renderer {

    VRenderer::VRenderer(const VulkanCore::VulkanInstance &instance, const VulkanCore::VDevice &device,
        const Client &client):m_device(device), m_client(client) {
        m_swapChain = std::make_unique<VulkanCore::VSwapChain>(device, instance);
        m_mainRenderPass = std::make_unique<VulkanCore::VRenderPass>(device, *m_swapChain);
        m_pipelineManager = std::make_unique<VulkanCore::VPipelineManager>(device, *m_swapChain, *m_mainRenderPass);
        m_pipelineManager->InstantiatePipelines();
        m_swapChain->CreateSwapChainFrameBuffers(*m_mainRenderPass);
        m_graphicsPipeline = &m_pipelineManager->GetPipeline(PIPELINE_TYPE_RASTER_BASIC);
        CreateCommandBufferPools();
        CreateSyncPrimitives();
    }

    void VRenderer::Render() {
        m_isFrameFinishFences[m_currentFrameIndex]->WaitForFence();
        //rerender the frame if image to present on is out of date
        if(FetchSwapChainImage() == vk::Result::eEventReset) return;
        m_isFrameFinishFences[m_currentFrameIndex]->ResetFence();
        m_baseCommandBuffers[m_currentFrameIndex]->Reset();
        RecordCommandBuffersForPipelines();
        SubmitCommandBuffer();
        PresentResults();
        m_currentFrameIndex = (m_currentImageIndex + 1) % GlobalVariables::MAX_FRAMES_IN_FLIGHT;
    }

    void VRenderer::CreateCommandBufferPools() {

        Utils::Logger::LogInfo("Allocating command pools...");
        m_baseCommandPool = std::make_unique<VulkanCore::VCommandPool>(m_device, QUEUE_FAMILY_INDEX_GRAPHICS);
        m_baseCommandBuffers.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i<GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
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
        renderPassBeginInfo.framebuffer = m_swapChain->GetSwapChainFrameBuffers()[m_currentImageIndex].get().GetFrameBuffer();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = m_swapChain->GetExtent();

        vk::ClearValue clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
    }

    void VRenderer::RecordCommandBuffersForPipelines() {
        m_baseCommandBuffers[m_currentFrameIndex]->BeginRecording();
        StartRenderPass();
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics,m_graphicsPipeline->GetPipelineInstance());
        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;

        viewport.width = static_cast<float>(m_swapChain->GetExtent().width);
        viewport.height = static_cast<float>(m_swapChain->GetExtent().height);

        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().setViewport(0,1, &viewport);

        vk::Rect2D scissors{};
        scissors.offset.x = 0;
        scissors.offset.y = 0;
        scissors.extent = m_swapChain->GetExtent();
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().setScissor(0,1, &scissors);

        auto mesh = m_client.GetMeshes()[0].get();
        std::vector<vk::Buffer> vertexBuffers = { mesh.GetVertexArray()->GetVertexBuffer().GetBuffer() };
        std::vector<vk::DeviceSize> offsets = {0};
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().bindIndexBuffer(mesh.GetVertexArray()->GetIndexBuffer().GetBuffer(),0, vk::IndexType::eUint32);
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().bindVertexBuffers(0,1,vertexBuffers.data(), offsets.data());
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().drawIndexed(mesh.GetMeshIndexCount(), 1, 0,0,0);

        EndRenderPass();
        m_baseCommandBuffers[m_currentFrameIndex]->EndRecording();
    }

    void VRenderer::EndRenderPass() {
        m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer().endRenderPass();
    }

    void VRenderer::CreateSyncPrimitives() {
        m_imageAvailableSemaphores.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_isFrameFinishFences.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i<GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
            m_imageAvailableSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
            m_renderFinishedSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
            m_isFrameFinishFences[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device, true);
        }
    }
    //===============================================================================================================


    //===============================================================================================================
    // FOR RENDER DRAWING
    //=================================================================================================================

    vk::Result VRenderer::FetchSwapChainImage() {

        auto imageIndex = m_device.GetDevice().acquireNextImageKHR(
            m_swapChain->GetSwapChain(), //swap chain
            UINT64_MAX, // timeoout
            m_imageAvailableSemaphores[m_currentFrameIndex]->GetSyncPrimitive()//signal semaphore,
            );
        switch (imageIndex.result) {
            case vk::Result::eSuccess: {
                m_currentImageIndex = imageIndex.value;
                Utils::Logger::LogInfoVerboseRendering("Swap chain is successfuly retrieved");
                return vk::Result::eSuccess;
            }
            case vk::Result::eErrorOutOfDateKHR: {
                m_swapChain->RecreateSwapChain();
                Utils::Logger::LogError("Swap chain was out of date ");
                return vk::Result::eEventReset;
            }
            case vk::Result::eSuboptimalKHR: {
                throw std::runtime_error("Suboptimal swap chain retrieved");
            };
            default: break;
        }
    }

    void VRenderer::SubmitCommandBuffer() {
        assert(!m_baseCommandBuffers[m_currentFrameIndex]->GetIsRecording());
        vk::SubmitInfo submitInfo;
        std::array<vk::Semaphore,1> waitSemaphores = { m_imageAvailableSemaphores[m_currentFrameIndex]->GetSyncPrimitive() };
        std::array<vk::PipelineStageFlags,1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_baseCommandBuffers[m_currentFrameIndex]->GetCommandBuffer();

        std::vector<vk::Semaphore> signalSemaphores = {m_renderFinishedSemaphores[m_currentFrameIndex]->GetSyncPrimitive()};
        submitInfo.signalSemaphoreCount = signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();

        assert(m_device.GetGraphicsQueue().submit( 1 ,&submitInfo, m_isFrameFinishFences[m_currentFrameIndex]->GetSyncPrimitive()) == vk::Result::eSuccess);
        Utils::Logger::LogInfoVerboseRendering("Successfully submitted the command buffer");
    }

    void VRenderer::PresentResults() {
        vk::PresentInfoKHR presentInfo;
        presentInfo.waitSemaphoreCount = 1 ;
        presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrameIndex]->GetSyncPrimitive();

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain->GetSwapChain();
        presentInfo.pImageIndices = &m_currentImageIndex;
        presentInfo.pResults = nullptr;
        vk::Result result;
        try{
            result = m_device.GetPresentQueue().presentKHR(&presentInfo);
        }catch (std::exception& e) {
            if(result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR) {
                m_swapChain->RecreateSwapChain();
            }
        }
        Utils::Logger::LogInfoVerboseRendering("Image presented to the view successfully");
    }

    void VRenderer::Destroy() {
        for(int i = 0; i<GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
            m_imageAvailableSemaphores[i]->Destroy();
            m_renderFinishedSemaphores[i]->Destroy();
            m_isFrameFinishFences[i]->Destroy();
            m_baseCommandBuffers[i]->Destroy();
        }
        m_mainRenderPass->Destroy();
        m_pipelineManager->DestroyPipelines();
        m_baseCommandPool->Destroy();
        m_swapChain->Destroy();
    }

} // Renderer
