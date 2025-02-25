//
// Created by wpsimon09 on 21/12/24.
//

#include "RenderingSystem.hpp"

#include "Editor/UIContext/UIContext.hpp"
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
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"


namespace Renderer {
    RenderingSystem::RenderingSystem(const VulkanCore::VulkanInstance& instance,const VulkanCore::VDevice& device,
        const VulkanUtils::VUniformBufferManager& uniformBufferManager,
         VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
         VEditor::UIContext &uiContext): m_device(device), m_uniformBufferManager(uniformBufferManager), m_pushDescriptorSetManager(pushDescriptorManager), m_renderContext(), m_uiContext(uiContext)
    {

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
            m_imageAvailableSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device, false);
            m_renderFinishedSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device, false);
            m_isFrameFinishFences[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Fence>>(m_device, true);
        }

        //----------------------------------------------------------------------------------------------------------------------------
        // Renderers creation
        //----------------------------------------------------------------------------------------------------------------------------
        m_sceneRenderer = std::make_unique<Renderer::SceneRenderer>(m_device, m_pushDescriptorSetManager, GlobalVariables::RenderTargetResolutionWidth , GlobalVariables::RenderTargetResolutionHeight);
        m_uiRenderer = std::make_unique<Renderer::UserInterfaceRenderer>(m_device, *m_swapChain, uiContext);

        //------------------------------------------------------------------------------------------------------------------------
        // CREATE PIPELINE MANAGER
        //------------------------------------------------------------------------------------------------------------------------
        m_pipelineManager = std::make_unique<VulkanCore::VPipelineManager>(m_device, *m_swapChain, m_sceneRenderer->GetRenderPass(0), m_pushDescriptorSetManager) ;
        m_pipelineManager->InstantiatePipelines();

        m_sceneRenderer->Init(m_pipelineManager.get());

        m_uiContext.GetViewPortContext(ViewPortType::eMain).currentFrameInFlight = m_currentFrameIndex;

        m_pushDescriptorSetManager.CreateUpdateTemplate(m_pipelineManager->GetPipeline(EPipelineType::DebugLines));

    }

    void RenderingSystem::Init()
    {
        for (int i = 0; i<GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_uiContext.GetViewPortContext(ViewPortType::eMain).SetImage(m_sceneRenderer->GetRenderedImage(m_currentFrameIndex), i);
        }
    }

    void RenderingSystem::Render(LightStructs::SceneLightInfo& sceneLightInfo,GlobalUniform& globalUniformUpdateInfo)
    {
        m_isFrameFinishFences[m_currentFrameIndex]->WaitForFence();

        //=================================================
        // GET SWAP IMAGE INDEX
        //=================================================
        auto imageIndex = VulkanUtils::SwapChainNextImageKHRWrapper(m_device, *m_swapChain,UINT64_MAX,
                                                                 *m_imageAvailableSemaphores[m_currentFrameIndex],
                                                                 nullptr);
        switch (imageIndex.first) {
        case vk::Result::eSuccess: {
                m_currentImageIndex = imageIndex.second;
                Utils::Logger::LogInfoVerboseRendering("Swap chain is successfuly retrieved");
                break;
        }
        case vk::Result::eErrorOutOfDateKHR: {
                m_swapChain->RecreateSwapChain();
                m_uiRenderer->GetRenderTarget().HandleSwapChainResize(*m_swapChain);
                Utils::Logger::LogError("Swap chain was out of date, trying to recreate it...  ");
                return;
        }
        case vk::Result::eSuboptimalKHR: {
                m_currentImageIndex = imageIndex.second;
                break;
                //m_swapChain->RecreateSwapChain();
                //return;
        };
        default:
            break;
        }


        m_isFrameFinishFences[m_currentFrameIndex]->ResetFence();

        m_uniformBufferManager.UpdatePerFrameUniformData(m_currentFrameIndex,globalUniformUpdateInfo);

        auto drawCalls = m_renderContext.GetAllDrawCall();
        m_uniformBufferManager.UpdatePerObjectUniformData(m_currentFrameIndex, drawCalls);

        m_uniformBufferManager.UpdateLightUniformData(m_currentFrameIndex, sceneLightInfo);

        // render scene
        m_sceneRenderer->Render(m_currentFrameIndex, m_uniformBufferManager, &m_renderContext);

                                                                    // semaphore signaled in the scene render pass
        std::vector<vk::Semaphore> waitSemaphoresForTransfering = {m_sceneRenderer->GetRendererFinishedSempahore(m_currentFrameIndex)};
        std::vector<vk::PipelineStageFlags> waitStagesForTransfering = {vk::PipelineStageFlagBits::eColorAttachmentOutput}; // what should be here  ?
        std::vector<vk::Semaphore> signalSemaphoresForTransfering = {m_renderFinishedSemaphores[m_currentFrameIndex]->GetSyncPrimitive()};

        // transition scene image layout
        m_sceneRenderer->GetRenderedImage(m_currentFrameIndex).TransitionImageLayout(vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
            waitSemaphoresForTransfering,
            waitStagesForTransfering,
            signalSemaphoresForTransfering);

        // gather all semaphores presentation should wait on
        std::vector<vk::Semaphore> waitSemaphoresForPresenting = {m_imageAvailableSemaphores[m_currentFrameIndex]->GetSyncPrimitive(),  m_renderFinishedSemaphores[m_currentFrameIndex]->GetSyncPrimitive()};
        std::vector<vk::PipelineStageFlags> waitStagesForPresenting = {vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader};

        // render UI and present to swap chain
        m_uiRenderer->RenderAndPresent(m_currentFrameIndex,m_currentImageIndex, *m_isFrameFinishFences[m_currentFrameIndex], waitSemaphoresForPresenting, waitStagesForPresenting );

        m_currentFrameIndex = (m_currentFrameIndex + 1) % GlobalVariables::MAX_FRAMES_IN_FLIGHT;
    }

    void RenderingSystem::Update()
    {
        m_renderContext.ResetAllDrawCalls();
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
        m_uiRenderer->Destroy();
        m_swapChain->Destroy();
        m_pipelineManager->DestroyPipelines();
    }
} // Renderer