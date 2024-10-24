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
        m_isFrameFinishFence->WaitForFence();
        FetchSwapChainImage();
        m_isFrameFinishFence->ResetFence();
        m_baseCommandBuffer->Reset();
        RecordCommandBuffersForPipelines();
        SubmitCommandBuffer();
        PresentResults();

    }

    void VRenderer::CreateCommandBufferPools() {
        auto pipelines = m_pipelineManager->GetAllPipelines();

        Utils::Logger::LogInfo("Allocating command pools...");
        m_baseCommandPool = std::make_unique<VulkanCore::VCommandPool>(m_device, QUEUE_FAMILY_INDEX_GRAPHICS);
        m_baseCommandBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_baseCommandPool);
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

        m_baseCommandBuffer->GetCommandBuffer().beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
    }

    void VRenderer::RecordCommandBuffersForPipelines() {
        m_baseCommandBuffer->BeginRecording();
        StartRenderPass();
        m_baseCommandBuffer->GetCommandBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics,m_graphicsPipeline->GetPipelineInstance());
        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;

        viewport.width = static_cast<float>(m_swapChain->GetExtent().width);
        viewport.height = static_cast<float>(m_swapChain->GetExtent().height);

        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        m_baseCommandBuffer->GetCommandBuffer().setViewport(0,1, &viewport);

        vk::Rect2D scissors{};
        scissors.offset.x = 0;
        scissors.offset.y = 0;
        scissors.extent = m_swapChain->GetExtent();
        m_baseCommandBuffer->GetCommandBuffer().setScissor(0,1, &scissors);

        auto mesh = m_client.GetMeshes()[0].get();
        std::vector<vk::Buffer> vertexBuffers = { mesh.GetVertexArray()->GetVertexBuffer().GetBuffer() };
        std::vector<vk::DeviceSize> offsets = {0};
        m_baseCommandBuffer->GetCommandBuffer().bindIndexBuffer(mesh.GetVertexArray()->GetIndexBuffer().GetBuffer(),0, vk::IndexType::eUint32);
        m_baseCommandBuffer->GetCommandBuffer().bindVertexBuffers(0,1,vertexBuffers.data(), offsets.data());
        m_baseCommandBuffer->GetCommandBuffer().drawIndexed(mesh.GetMeshIndexCount(), 1, 0,0,0);

        EndRenderPass();
        m_baseCommandBuffer->EndRecording();
    }

    void VRenderer::EndRenderPass() {
        m_baseCommandBuffer->GetCommandBuffer().endRenderPass();
    }

    void VRenderer::CreateSyncPrimitives() {
        m_imageAvailableSemaphore = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
        m_renderFinishedSemaphore = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
        m_isFrameFinishFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device, true);
    }
    //===============================================================================================================


    //===============================================================================================================
    // FOR RENDER DRAWING
    //=================================================================================================================

    void VRenderer::FetchSwapChainImage() {

        auto imageIndex = m_device.GetDevice().acquireNextImageKHR(
            m_swapChain->GetSwapChain(), //swap chain
            UINT64_MAX, // timeoout
            m_imageAvailableSemaphore->GetSyncPrimitive()//signal semaphore,
            );
        switch (imageIndex.result) {
            case vk::Result::eSuccess: m_currentImageIndex = imageIndex.value; break;
            case vk::Result::eErrorOutOfDateKHR: throw std::runtime_error("Swap chain returned out of date, we have to implement resizing of the frame buffer "); break;
            default: break;
        }
    }

    void VRenderer::SubmitCommandBuffer() {
        assert(!m_baseCommandBuffer->GetIsRecording());
        vk::SubmitInfo submitInfo;
        std::array<vk::Semaphore,1> waitSemaphores = { m_imageAvailableSemaphore->GetSyncPrimitive() };
        std::array<vk::PipelineStageFlags,1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_baseCommandBuffer->GetCommandBuffer();

        std::vector<vk::Semaphore> signalSemaphores = {m_renderFinishedSemaphore->GetSyncPrimitive()};
        submitInfo.signalSemaphoreCount = signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();

        assert(m_device.GetGraphicsQueue().submit( 1 ,&submitInfo, m_isFrameFinishFence->GetSyncPrimitive()) == vk::Result::eSuccess);
        Utils::Logger::LogInfoVerboseRendering("Successfully submitted the command buffer");
    }

    void VRenderer::PresentResults() {
        vk::PresentInfoKHR presentInfo;
        presentInfo.waitSemaphoreCount = 1 ;
        presentInfo.pWaitSemaphores = &m_renderFinishedSemaphore->GetSyncPrimitive();

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain->GetSwapChain();
        presentInfo.pImageIndices = &m_currentImageIndex;
        presentInfo.pResults = nullptr;
        assert(m_device.GetPresentQueue().presentKHR(&presentInfo) == vk::Result::eSuccess);
        Utils::Logger::LogInfoVerboseRendering("Image presented to the view successfully");
    }

    void VRenderer::Destroy() {
        m_imageAvailableSemaphore->Destroy();
        m_renderFinishedSemaphore->Destroy();
        m_isFrameFinishFence->Destroy();
        m_mainRenderPass->Destroy();
        m_pipelineManager->DestroyPipelines();
        m_baseCommandBuffer->Destroy();
        m_baseCommandPool->Destroy();
        m_swapChain->Destroy();
    }

} // Renderer
