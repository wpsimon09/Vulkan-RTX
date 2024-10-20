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
        CreateCommandBufferPools();
        CreateSyncPrimitives();

        m_vertexBuffer_GPU = std::make_unique<VulkanCore::VBuffer>(m_device);
        m_vertexBuffer_GPU->MakeVertexBuffer(m_client.GetMeshes()[0]);
    }

    void VRenderer::Render() {
        m_isFrameFinishFence->WaitForFence();
        FetchSwapChainImage();
        RecordCommandBuffersForPipelines();
        m_isFrameFinishFence->ResetFence();
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
        for (auto &pipeline : m_pipelineManager->GetAllPipelines()) {
            pipeline.get().RecordPipelineCommands(*m_baseCommandBuffer);
        }
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
            m_imageAvailableSemaphore->GetSyncPrimitive()//signal semaphore
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

    }

    void VRenderer::Destroy() {
        m_vertexBuffer_GPU->Destroy();
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
