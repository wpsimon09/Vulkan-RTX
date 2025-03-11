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
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

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
        uiContext.Initialize(swapChain);
    }

    void UserInterfaceRenderer::RenderAndPresent(int currentFrameIndex, uint32_t swapChainImageIndex,const vk::Semaphore& swapChainImageAvailable,
        VulkanCore::VTimelineSemaphore& renderingTimeLine)
    {
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

        std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput };

        std::vector<vk::Semaphore> waitSemaphores = {renderingTimeLine.GetSemaphore(), swapChainImageAvailable};
        std::vector<vk::Semaphore> signalSemaphores = {renderingTimeLine.GetSemaphore(), m_ableToPresentSemaphore[currentFrameIndex]->GetSyncPrimitive()};

        vk::SubmitInfo submitInfo;
        auto next = renderingTimeLine.GetSemaphoreSubmitInfo(4, 6);

        std::vector<uint64_t> waitValues = {renderingTimeLine.GetCurrentWaitValue(), 20};
        std::vector<uint64_t> signalValues = {renderingTimeLine.GetCurrentSignalValue(), 21};

        next.pWaitSemaphoreValues = waitValues.data();
        next.waitSemaphoreValueCount = waitValues.size();

        next.signalSemaphoreValueCount = signalValues.size();
        next.pSignalSemaphoreValues = signalValues.data();

        submitInfo.pNext = &next;
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.waitSemaphoreCount = waitSemaphores.size();

        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.signalSemaphoreCount = signalValues.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffer[currentFrameIndex]->GetCommandBuffer();

        auto result = m_device.GetGraphicsQueue().submit(1, &submitInfo, nullptr);
        assert(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR);

        //===========================
        // PRESENT TO SCREEN
        //===========================
        vk::PresentInfoKHR presentInfo;
        //next = renderingTimeLine.GetSemaphoreSubmitInfo(6, 8);
        //presentInfo.pNext = &next;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_ableToPresentSemaphore[currentFrameIndex]->GetSyncPrimitive();

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain.GetSwapChain();
        presentInfo.pImageIndices = &swapChainImageIndex;

        presentInfo.pResults = nullptr;

        vk::Result presentResult = VulkanUtils::PresentQueueWrapper(m_device.GetPresentQueue(), presentInfo);

        renderingTimeLine.CpuWaitIdle(6);
        renderingTimeLine.CpuSignal(8);
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


        vk::RenderingInfo renderingInfo;
        renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderingInfo.renderArea.extent = vk::Extent2D(m_swapChain.GetExtent().width, m_swapChain.GetExtent().height);
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &m_renderTarget->GetColourAttachmentOneSample(swapChainImageIndex);;
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

        cmdBuffer.endRendering();

        VulkanUtils::RecordImageTransitionLayoutCommand(m_renderTarget->GetColourImage(swapChainImageIndex), vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eColorAttachmentOptimal, *m_commandBuffer[currentFrameIndex]);
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

