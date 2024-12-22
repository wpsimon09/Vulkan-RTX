//
// Created by wpsimon09 on 22/12/24.
//

#include "RenderTarget.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"


namespace Renderer {
    RenderTarget::RenderTarget(const VulkanCore::VDevice& device, int width, int height, vk::Format colourFormat):
        m_device(device)
    {

        Utils::Logger::LogInfoVerboseOnly("Creating render target...");

        m_colourBuffer.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_frameBuffers.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);

        m_depthBuffer = std::make_unique<VulkanCore::VImage>(m_device, 1, m_device.GetDepthFormat(), vk::ImageAspectFlagBits::eDepth);
        m_depthBuffer->Resize(width, height);
        m_depthBuffer->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_colourBuffer[i] = std::make_unique<VulkanCore::VImage>(m_device);
            m_colourBuffer[i]->Resize(width, height);
            m_colourBuffer[i]->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
        }

        m_renderPass = std::make_unique<VulkanCore::VRenderPass>(m_device,*m_colourBuffer[0],*m_depthBuffer ,false);

        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::vector<std::reference_wrapper<const VulkanCore::VImage>> attachments;
            attachments.emplace_back(*m_colourBuffer[i]);
            attachments.emplace_back(*m_depthBuffer);
            m_frameBuffers[i] = std::make_unique<VulkanCore::VFrameBuffer>(m_device, *m_renderPass,attachments, width, height);
        }

        Utils::Logger::LogSuccess("Render target created, Contains 2 colour buffers and 1 depth buffer");
    }

    void RenderTarget::HandleResize(int newWidth, int newHeight)
    {
    }

    void RenderTarget::Destroy()
    {
    }

    void RenderTarget::DestroyForResize()
    {
    }

} // Renderer