//
// Created by wpsimon09 on 21/12/24.
//

#include "UserInterfaceRenderer.hpp"

#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/Utils/VImGuiInitializer/ImGuiInitializer.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"

Renderer::UserInterfaceRenderer::UserInterfaceRenderer(const VulkanCore::VDevice& device,
    VulkanCore::VSwapChain* swapChain, VulkanUtils::ImGuiInitializer& imGuiInitilaizer): m_device(device),
    m_imguiInitializer(imGuiInitilaizer)
{
    CreateRenderTargets(swapChain);
    imGuiInitilaizer.Initialize(*m_renderTarget->m_renderPass);
}

void Renderer::UserInterfaceRenderer::RenderAndPresnet(int currentFrameIndex)
{
}

void Renderer::UserInterfaceRenderer::CreateRenderTargets(VulkanCore::VSwapChain* swapChain)
{
    m_renderTarget = std::make_unique<Renderer::RenderTarget>(m_device, swapChain->GetSwapChainImages(), swapChain->GetSurfaceFormatKHR().format, swapChain->GetExtent());
}
