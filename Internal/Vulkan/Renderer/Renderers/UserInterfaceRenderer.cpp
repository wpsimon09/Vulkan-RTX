//
// Created by wpsimon09 on 21/12/24.
//

#include "UserInterfaceRenderer.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/Utils/VImGuiInitializer/ImGuiInitializer.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"

Renderer::UserInterfaceRenderer::UserInterfaceRenderer(const VulkanCore::VDevice& device,
    const VulkanCore::VSwapChain& swapChain, VulkanUtils::ImGuiInitializer& imGuiInitilaizer): m_device(device),
    m_imguiInitializer(imGuiInitilaizer)
{
    m_commandPool = std::make_unique<VulkanCore::VCommandPool>(device, QUEUE_FAMILY_INDEX_GRAPHICS);
    m_commandBuffer.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_ableToPresentSemaphore.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    for (int i =0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_commandBuffer[i] = std::make_unique<VulkanCore::VCommandBuffer>(device, *m_commandPool, true );
        m_ableToPresentSemaphore[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(device, false);
    }

    m_renderTarget = std::make_unique<Renderer::RenderTarget>(m_device, swapChain);
    imGuiInitilaizer.Initialize(*m_renderTarget->m_renderPass);
}

void Renderer::UserInterfaceRenderer::RenderAndPresent(int currentFrameIndex, int swapChainImageIndex,
    const VulkanCore::VSyncPrimitive<vk::Fence>& renderingFinishedFence,
    std::vector<vk::Semaphore>& waitSemaphores, std::vector<vk::PipelineStageFlags>& pipelineStages)
{
    //=============================
    // RECORD CMD BUFFER
    //=============================
    m_commandBuffer[currentFrameIndex]->BeginRecording();
    RecordCommandBuffer(currentFrameIndex);
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

    auto result = m_device.GetGraphicsQueue().submit(1, &submitInfo, nullptr);

    assert(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR);

    //===========================
    // PRESENT TO SCREEN
    //===========================
    //TODO: tmrw

}

void Renderer::UserInterfaceRenderer::RecordCommandBuffer(int currentFrameIndex)
{

    //==============================================
    // CREATE RENDER PASS INFO
    //==============================================
    vk::RenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.renderPass = m_renderTarget->m_renderPass->GetRenderPass();
    renderPassBeginInfo.framebuffer = m_renderTarget->m_frameBuffers[currentFrameIndex]->GetFrameBuffer();
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>(m_renderTarget->m_width),
    renderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(m_renderTarget->m_height);

    //==============================================
    // CONFIGURE CLEAR
    //==============================================
    std::array<vk::ClearValue,2> clearColors = {};
    clearColors[0].color =  {0.2f, 0.2f, 0.2f, 1.0f};
    clearColors[1].depthStencil.depth = 1.0f;
    clearColors[1].depthStencil.stencil = 0.0f;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassBeginInfo.pClearValues = clearColors.data();

    //==============================================
    // START RENDER PASS
    //==============================================
    assert(m_commandBuffer[currentFrameIndex]->GetIsRecording());
    auto& cmdBuffer = m_commandBuffer[currentFrameIndex]->GetCommandBuffer();


    m_imguiInitializer.BeginRender();
    cmdBuffer.beginRenderPass(
        &renderPassBeginInfo, vk::SubpassContents::eInline);

    m_imguiInitializer.Render(*m_commandBuffer[currentFrameIndex]);

    cmdBuffer.endRenderPass();

}

