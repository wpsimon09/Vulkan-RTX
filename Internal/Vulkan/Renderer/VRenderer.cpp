//
// Created by wpsimon09 on 15/10/24.
//

#include "VRenderer.hpp"
#include "VRenderer.hpp"

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
        m_renderingCommandPool = std::make_unique<VulkanCore::VCommandPool>(device, QUEUE_FAMILY_INDEX_GRAPHICS);
        m_renderingCommandBuffer = std::make_unique<VulkanCore::VCommandBuffer>(device, *m_renderingCommandPool);
        CreateSyncPrimitives();
    }

    void VRenderer::Render() {
        m_isFrameFinishFence->WaitForFence();

        FetchSwapChainImage();
        m_isFrameFinishFence->ResetFence();
    
        m_renderingCommandBuffer->GetCommandBuffer().reset();
        for (auto tzname : m_pipelineManager->Get) {
            
        }
        
    }

    //==============================================================================
    // FOR COMMAND BUFFER
    //==============================================================================
    void VRenderer::RecordCommandBuffer(const VulkanCore::VGraphicsPipeline& pipeline) {
        m_renderingCommandBuffer->BeginRecording();
        StartRenderPass();
        PrepareViewPort(pipeline);
        Draw(pipeline);
        m_renderingCommandBuffer->EndRecording();
    }

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

        m_renderingCommandBuffer->GetCommandBuffer().beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
    }


    void VRenderer::EndRenderPass() {
        m_renderingCommandBuffer->GetCommandBuffer().endRenderPass();
    }

    void VRenderer::CreateSyncPrimitives() {
        m_imageAvailableSemaphore = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
        m_renderFinishedSemaphore = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
        m_isFrameFinishFence = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device, true);
    }


    void VRenderer::PrepareViewPort(const VulkanCore::VGraphicsPipeline& pipeline) {
        m_renderingCommandBuffer->GetCommandBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineInstance() );

        vk::Viewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain->GetExtent().width);
        viewport.height = static_cast<float>(m_swapChain->GetExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        m_renderingCommandBuffer->GetCommandBuffer().setViewport(0,1, &viewport);

        vk::Rect2D scissors;
        scissors.offset.x = 0;
        scissors.offset.y = 0;
        scissors.extent = m_swapChain->GetExtent();
        m_renderingCommandBuffer->GetCommandBuffer().setScissor(0,1, &scissors);
    }

    void VRenderer::Draw(const VulkanCore::VGraphicsPipeline& pipeline) {
        m_renderingCommandBuffer->GetCommandBuffer().draw(3,1,0,0);
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

    void VRenderer::Destroy() {
        m_imageAvailableSemaphore->Destroy();
        m_renderFinishedSemaphore->Destroy();
        m_isFrameFinishFence->Destroy();
        m_mainRenderPass->Destroy();
        m_pipelineManager->DestroyPipelines();
        m_renderingCommandBuffer->Destroy();
        m_renderingCommandPool->Destroy();
        m_swapChain->Destroy();
    }

} // Renderer
