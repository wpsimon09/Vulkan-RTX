//
// Created by wpsimon09 on 21/12/24.
//

#include "RenderingSystem.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/Renderer/Renderers/UserInterfaceRenderer.hpp"
#include "Vulkan/Renderer/Renderers/SceneRenderer.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"

namespace Renderer {
    RenderingSystem::RenderingSystem(VulkanCore::VulkanInstance instance,const VulkanCore::VDevice& device,
        const VulkanUtils::VUniformBufferManager& uniformBufferManager,
         VulkanUtils::VPushDescriptorManager& pushDescriptorManager): m_device(device), m_uniformBufferManager(uniformBufferManager), m_pushDescriptorSetManager(pushDescriptorManager)
    {
        //------------------------------------------------------------------------------------------------------------------------
        // CREATE SYNCHRONIZATION PRIMITIVES
        //------------------------------------------------------------------------------------------------------------------------
        m_imageAvailableSemaphores.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_isFrameFinishFences.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
            m_imageAvailableSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
            m_renderFinishedSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
            m_isFrameFinishFences[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device, true);
        }

        //---------------------------------------------------------------------------------------------------------------------------
        // Swap chain creation
        //---------------------------------------------------------------------------------------------------------------------------
        m_swapChain = std::make_unique<VulkanCore::VSwapChain>(m_device, instance);

    }
} // Renderer