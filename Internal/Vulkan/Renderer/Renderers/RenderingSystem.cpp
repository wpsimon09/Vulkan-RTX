//
// Created by wpsimon09 on 21/12/24.
//

#include "RenderingSystem.hpp"

#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Lightning/LightStructs.hpp"
#include "Application/Utils/ApplicationUtils.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"

#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/Renderer/Renderers/UserInterfaceRenderer.hpp"
#include "Vulkan/Renderer/Renderers/SceneRenderer.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/Utils/VEnvLightGenerator/VEnvLightGenerator.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"




namespace Renderer {
    RenderingSystem::RenderingSystem(const VulkanCore::VulkanInstance& instance,const VulkanCore::VDevice& device,
        const VulkanUtils::VUniformBufferManager& uniformBufferManager,
         VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
         VEditor::UIContext &uiContext)
    : m_device(device), m_uniformBufferManager(uniformBufferManager), m_pushDescriptorSetManager(pushDescriptorManager), m_renderContext(), m_uiContext(uiContext), m_transferSemapohore(device.GetTransferOpsManager().GetTransferSemaphore())
    {

        //---------------------------------------------------------------------------------------------------------------------------
        // Swap chain creation
        //---------------------------------------------------------------------------------------------------------------------------
        m_swapChain = std::make_unique<VulkanCore::VSwapChain>(m_device, instance);

        //------------------------------------------------------------------------------------------------------------------------
        // CREATE SYNCHRONIZATION PRIMITIVES
        //------------------------------------------------------------------------------------------------------------------------
        m_renderingTimeLine.resize(2);
        m_imageAvailableSemaphores.resize(2);
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
            m_renderingTimeLine[i] = std::make_unique<VulkanCore::VTimelineSemaphore>(m_device, 8);
            m_imageAvailableSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>> (m_device);
        }

        //----------------------------------------------------------------------------------------------------------------------------
        // Renderers creation
        //----------------------------------------------------------------------------------------------------------------------------
        m_sceneRenderer = std::make_unique<Renderer::SceneRenderer>(
            m_device,
            m_pushDescriptorSetManager,
            GlobalVariables::RenderTargetResolutionWidth ,
            GlobalVariables::RenderTargetResolutionHeight);

        m_uiRenderer = std::make_unique<Renderer::UserInterfaceRenderer>(
            m_device,
            *m_swapChain,
            uiContext);

        m_uiContext.GetViewPortContext(ViewPortType::eMain).currentFrameInFlight = m_currentFrameIndex;

        m_envLightGenerator = std::make_unique<VulkanUtils::VEnvLightGenerator>(m_device, pushDescriptorManager);

        Utils::Logger::LogInfo("RenderingSystem initialized");

    }

    void RenderingSystem::Init()
    {
        for (int i = 0; i<GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_uiContext.GetViewPortContext(ViewPortType::eMain).SetImage(m_sceneRenderer->GetRenderedImage(m_currentFrameIndex), i);
           //m_uiContext.GetViewPortContext(ViewPortType::eMain).SetImage(m_envLightGenerator->GetBRDFLut(), i);
        }
    }

    void RenderingSystem::Render(LightStructs::SceneLightInfo& sceneLightInfo,GlobalUniform& globalUniformUpdateInfo)
    {
        m_renderingTimeLine[m_currentFrameIndex]->CpuWaitIdle(8);
        m_sceneLightInfo = &sceneLightInfo;
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


        m_renderingTimeLine[m_currentFrameIndex]->Reset();

        m_device.GetTransferOpsManager().UpdateGPU();

        // ==== check if it is possible ot use env light
        m_uniformBufferManager.UpdatePerFrameUniformData(m_currentFrameIndex,globalUniformUpdateInfo);
        m_uniformBufferManager.UpdateLightUniformData(m_currentFrameIndex, sceneLightInfo);
        m_uniformBufferManager.UpdatePerObjectUniformData(m_currentFrameIndex, m_renderContext.GetAllDrawCall());

        std::sort(m_renderContext.drawCalls.begin(), m_renderContext.drawCalls.end(), [](std::pair<unsigned long, VulkanStructs::DrawCallData> & lhs,std::pair<unsigned long, VulkanStructs::DrawCallData> & rhs )
        {
            return lhs.first < rhs.first;
        });

        // generate new IBL maps if new one was selected
        if (sceneLightInfo.environmentLight != nullptr)
            if (sceneLightInfo.environmentLight->hdrImage->IsAvailable())
                m_envLightGenerator->Generate(sceneLightInfo.environmentLight->hdrImage->GetHandle(), *m_renderingTimeLine[m_currentFrameIndex]);
        m_renderContext.hdrCubeMap =    m_envLightGenerator->GetCubeMapRaw();
        m_renderContext.irradianceMap = m_envLightGenerator->GetIrradianceMapRaw();
        m_renderContext.prefilterMap =  m_envLightGenerator->GetPrefilterMapRaw();
        m_renderContext.brdfMap =       m_envLightGenerator->GetBRDFLutRaw();
        m_renderContext.dummyCubeMap =  m_envLightGenerator->GetDummyCubeMapRaw();
        // render scene
        m_sceneRenderer->Render(m_currentFrameIndex, m_uniformBufferManager, &m_renderContext, *m_renderingTimeLine[m_currentFrameIndex], m_transferSemapohore);

        // render UI and present to swap chain
        m_uiRenderer->RenderAndPresent(m_currentFrameIndex,m_currentImageIndex,m_imageAvailableSemaphores[m_currentFrameIndex]->GetSyncPrimitive(), *m_renderingTimeLine[m_currentFrameIndex]);

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
        }
        m_sceneRenderer->Destroy();
        m_uiRenderer->Destroy();
        m_swapChain->Destroy();
        m_envLightGenerator->Destroy();
    }
} // Renderer