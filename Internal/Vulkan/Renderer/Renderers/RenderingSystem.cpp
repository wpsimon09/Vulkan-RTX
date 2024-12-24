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
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Pipeline/VPipelineManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"


namespace Renderer {
    RenderingSystem::RenderingSystem(const VulkanCore::VulkanInstance& instance,const VulkanCore::VDevice& device,
        const VulkanUtils::VUniformBufferManager& uniformBufferManager,
         VulkanUtils::VPushDescriptorManager& pushDescriptorManager): m_device(device), m_uniformBufferManager(uniformBufferManager), m_pushDescriptorSetManager(pushDescriptorManager), m_mainRenderContext{}
    {
        m_renderingContext = &m_mainRenderContext;

        //---------------------------------------------------------------------------------------------------------------------------
        // Swap chain creation
        //---------------------------------------------------------------------------------------------------------------------------
        m_swapChain = std::make_unique<VulkanCore::VSwapChain>(m_device, instance);

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

        //----------------------------------------------------------------------------------------------------------------------------
        // Renderers creation
        //----------------------------------------------------------------------------------------------------------------------------
        m_sceneRenderer = std::make_unique<Renderer::SceneRenderer>(m_device, m_pushDescriptorSetManager, 800, 600);

        //------------------------------------------------------------------------------------------------------------------------
        // CREATE PIPELINE MANAGER
        //------------------------------------------------------------------------------------------------------------------------
        m_pipelineManager = std::make_unique<VulkanCore::VPipelineManager>(m_device, *m_swapChain, m_sceneRenderer->GetRenderPass(0), m_pushDescriptorSetManager) ;
        m_pipelineManager->InstantiatePipelines();

        //m_pushDescriptorSetManager.CreateUpdateTemplate(m_pipelineManager->GetPipeline(PIPELINE_TYPE::PIPELINE_TYPE_RASTER_PBR_TEXTURED));
    }

    void RenderingSystem::Render(GlobalUniform& globalUniformUpdateInfo)
    {
        m_isFrameFinishFences[m_currentFrameIndex]->WaitForFence();
        m_isFrameFinishFences[m_currentFrameIndex]->ResetFence();

        m_uniformBufferManager.UpdatePerFrameUniformData(m_currentFrameIndex,globalUniformUpdateInfo);
        m_uniformBufferManager.UpdatePerObjectUniformData(m_currentFrameIndex, m_renderingContext->DrawCalls);

        // render scene
        m_sceneRenderer->Render(m_currentFrameIndex,globalUniformUpdateInfo, *m_renderingContext, m_pipelineManager->GetPipeline(PIPELINE_TYPE::PIPELINE_TYPE_RASTER_PBR_TEXTURED)  );

        // render UI

        // present swap chain

        m_currentFrameIndex = (m_currentImageIndex + 1) % GlobalVariables::MAX_FRAMES_IN_FLIGHT;
    }

    void RenderingSystem::Update()
    {

    }

    void RenderingSystem::Destroy()
    {
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_imageAvailableSemaphores[i]->Destroy();
            m_renderFinishedSemaphores[i]->Destroy();
            m_isFrameFinishFences[i]->Destroy();
        }
        m_sceneRenderer->Destroy();
        m_pipelineManager->DestroyPipelines();
    }
} // Renderer