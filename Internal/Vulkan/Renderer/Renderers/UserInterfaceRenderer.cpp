//
// Created by wpsimon09 on 21/12/24.
//

#include "UserInterfaceRenderer.hpp"

#include <Vulkan/Utils/VIimageTransitionCommands.hpp>

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"

#include "Editor/UIContext/UIContext.hpp"

namespace Renderer
{

    UserInterfaceRenderer::UserInterfaceRenderer(const VulkanCore::VDevice& device,
        const VulkanCore::VSwapChain& swapChain, VEditor::UIContext& uiContext): m_device(device),
        m_imguiInitializer(uiContext), m_swapChain(swapChain)
    {
        m_commandPool = std::make_unique<VulkanCore::VCommandPool>(device, Graphics);
        m_commandBuffer.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_ableToPresentSemaphore.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for (int i =0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_commandBuffer[i] = std::make_unique<VulkanCore::VCommandBuffer>(device, *m_commandPool, true );
            m_ableToPresentSemaphore[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(device, false);
        }

        m_renderTarget = std::make_unique<Renderer::RenderTarget>(m_device, swapChain);
        uiContext.Initialize();
    }

    void UserInterfaceRenderer::RenderAndPresent(int currentFrameIndex, uint32_t swapChainImageIndex,
                                                           const VulkanCore::VSyncPrimitive<vk::Fence>& renderingFinishedFence,
                                                           std::vector<vk::Semaphore>& waitSemaphores, std::vector<vk::PipelineStageFlags>& pipelineStages)
    {

        //===========================================================
        // CONVERT IMAGE LAYOUT FROM KHR_PRESENT TO COLOUR_ATTACHMENT
        //===========================================================
        auto transitionCommandBuffer = VulkanCore::VCommandBuffer(m_device, m_device.GetTransferCommandPool());
        auto transitionFinishedSemaphore = VulkanCore::VSyncPrimitive<vk::Semaphore>(m_device);


        //=============================
        // RECORD CMD BUFFER
        //=============================
        m_commandBuffer[currentFrameIndex]->BeginRecording();

        RecordCommandBuffer(currentFrameIndex, swapChainImageIndex);

        m_commandBuffer[currentFrameIndex]->EndRecording();
        m_imguiInitializer.EndRender();

        //===========================
        // SUBMIT THE WORK
        //===========================
        assert(!m_commandBuffer[currentFrameIndex]->GetIsRecording());


        vk::SubmitInfo submitInfo;
        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = pipelineStages.data();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffer[currentFrameIndex]->GetCommandBuffer();

        std::vector<vk::Semaphore> signalSemaphores = {
            m_ableToPresentSemaphore[currentFrameIndex]->GetSyncPrimitive()
        };

        submitInfo.signalSemaphoreCount = signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();
        auto result = m_device.GetGraphicsQueue().submit(1, &submitInfo, renderingFinishedFence.GetSyncPrimitive());
        assert(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR);

        //===========================
        // PRESENT TO SCREEN
        //===========================
        vk::PresentInfoKHR presentInfo;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_ableToPresentSemaphore[currentFrameIndex]->GetSyncPrimitive();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain.GetSwapChain();
        presentInfo.pImageIndices = &swapChainImageIndex;
        presentInfo.pResults = nullptr;
        vk::Result presentResult = VulkanUtils::PresentQueueWrapper(m_device.GetPresentQueue(), presentInfo);
        //assert(presentResult == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR);
    }

    void UserInterfaceRenderer::RecordCommandBuffer(int currentFrameIndex, uint32_t swapChainImageIndex)
    {

        //==============================================
        // CREATE RENDER PASS INFO
        //==============================================
        VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTarget->GetColourImage(swapChainImageIndex), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eUndefined, *m_commandBuffer[currentFrameIndex]);

        //==============================================
        // CREATE RENDER PASS INFO
        //==============================================
        std::vector<vk::RenderingAttachmentInfo> colourAttachments = {
            m_renderTarget->GetColourAttachment(currentFrameIndex),
        };

        vk::RenderingInfo renderingInfo;
        renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderingInfo.renderArea.extent = vk::Extent2D(m_swapChain.GetExtent().width, m_swapChain.GetExtent().height);
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = colourAttachments.size();
        renderingInfo.pColorAttachments = colourAttachments.data();
        renderingInfo.pDepthAttachment = &m_renderTarget->GetDepthAttachment();

        //==============================================
        // START RENDER PASS
        //==============================================
        auto& cmdBuffer = m_commandBuffer[currentFrameIndex]->GetCommandBuffer();

        cmdBuffer.beginRendering(&renderingInfo);

        //==============================================
        // START RENDER PASS
        //==============================================

        assert(m_commandBuffer[currentFrameIndex]->GetIsRecording());
        m_imguiInitializer.Render(*m_commandBuffer[currentFrameIndex]);

        VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTarget->GetColourImage(swapChainImageIndex), vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eColorAttachmentOptimal, *m_commandBuffer[currentFrameIndex]);


        cmdBuffer.endRenderPass();
    }

    void UserInterfaceRenderer::Destroy()
    {
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_ableToPresentSemaphore[i]->Destroy();
        }
        m_renderTarget->Destroy();
        m_commandPool->Destroy();
    }
}

