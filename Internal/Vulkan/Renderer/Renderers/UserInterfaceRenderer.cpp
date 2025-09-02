//
// Created by wpsimon09 on 21/12/24.
//

#include "UserInterfaceRenderer.hpp"


#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"

#include "Editor/UIContext/UIContext.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

namespace Renderer {

UserInterfaceRenderer::UserInterfaceRenderer(const VulkanCore::VDevice&    device,
                                             const VulkanCore::VSwapChain& swapChain,
                                             VEditor::UIContext&           uiContext)
    : m_device(device)
    , m_imguiInitializer(uiContext)
    , m_swapChain(swapChain)
{

    m_renderTarget = std::make_unique<Renderer::RenderTarget2>(m_device, swapChain);
    uiContext.Initialize(swapChain);
}

void UserInterfaceRenderer::Render(int currentFrameIndex, uint32_t swapChainImageIndex, VulkanCore::VCommandBuffer& cmdBuffer)
{

    assert(cmdBuffer.GetIsRecording() && "Command buffer is not recording !");
    //==============================================
    // CREATE RENDER PASS INFO
    //==============================================
    VulkanUtils::VBarrierPosition barrierPosition{{},
                                                  {},
                                                  vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                  vk::AccessFlagBits2::eColorAttachmentWrite};

    VulkanUtils::PlaceImageMemoryBarrier2(m_renderTarget->GetSwapChainImage(swapChainImageIndex), cmdBuffer,
                                          vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eAttachmentOptimalKHR, barrierPosition);

    //==============================================
    // CREATE RENDER PASS INFO
    //==============================================


    auto colourAttachment = m_renderTarget->GenerateAttachmentInfoForSwapChain(swapChainImageIndex);
    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_swapChain.GetExtent().width, m_swapChain.GetExtent().height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments    = &colourAttachment;
    renderingInfo.pDepthAttachment = {};

    //==============================================
    // START RENDER PASS
    //==============================================
    auto& cmdB = cmdBuffer.GetCommandBuffer();

    cmdB.beginRendering(&renderingInfo);

    //==============================================
    // START RENDER PASS
    //==============================================
    m_imguiInitializer.Render(cmdBuffer);

    cmdB.endRendering();

    barrierPosition = {vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                  vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2};
    VulkanUtils::PlaceImageMemoryBarrier2(m_renderTarget->GetSwapChainImage(swapChainImageIndex), cmdBuffer,
                                          vk::ImageLayout::eAttachmentOptimalKHR, vk::ImageLayout::ePresentSrcKHR, barrierPosition);

    m_imguiInitializer.EndRender();

    //===========================
    // SUBMIT THE WORK
    //===========================

    //assert(presentResult == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR);
}
void UserInterfaceRenderer::Present(uint32_t                        swapChainImageIndex,
                                    const vk::Semaphore&            ableToPresentSemaphore)
{
    //===========================
    // PRESENT TO SCREEN
    //===========================

    vk::PresentInfoKHR presentInfo;
    //auto next = renderingTimeLine.GetTimeLineSemaphoreSubmitInfo(4, 8);
    //presentInfo.pNext = &next;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &ableToPresentSemaphore;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains    = &m_swapChain.GetSwapChain();
    presentInfo.pImageIndices  = &swapChainImageIndex;

    presentInfo.pResults = nullptr;

    vk::Result presentResult = VulkanUtils::PresentQueueWrapper(m_device.GetPresentQueue(), presentInfo);
}


void UserInterfaceRenderer::HandleSwapChainResize(VulkanCore::VSwapChain& resizedSwapChain) {
    m_renderTarget->Destroy();
    m_renderTarget = std::make_unique<RenderTarget2>(m_device, resizedSwapChain);
}
void UserInterfaceRenderer::Destroy()
{

    m_renderTarget->Destroy();
}
}  // namespace Renderer
