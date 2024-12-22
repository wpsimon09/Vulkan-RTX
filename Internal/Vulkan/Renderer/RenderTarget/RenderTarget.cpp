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
        m_colourBuffer.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_frameBuffers.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_colourBuffer[i] = std::make_unique<VulkanCore::VImage>(m_device);
            m_colourBuffer[i]->Resize(width, height);
            m_colourBuffer[i]->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
        }
    }

    void RenderTarget::Finalize(int frameIndex)
    {
        m_colourBuffer[frameIndex]->TransitionImageLayout(vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    }
} // Renderer