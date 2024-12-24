//
// Created by wpsimon09 on 23/12/24.
//

#include "BaseRenderer.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


namespace Renderer {
    BaseRenderer::BaseRenderer(const VulkanCore::VDevice& device):m_device(device){
        m_commandBuffers.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_rendererFinishedSemaphore.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_rendererFinishedSemaphore[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device, true);
        }
    }

    void BaseRenderer::Destroy()
    {
        for (uint32_t i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_commandBuffers[i]->Destroy();
            m_renderTargets[i]->Destroy();
            m_rendererFinishedSemaphore[i]->Destroy();
        }
    }
} // Renderer