//
// Created by wpsimon09 on 21/12/24.
//

#include "RenderingSystem.hpp"

#include "Application/ApplicationState/ApplicationState.hpp"
#include "Editor/UIContext/ViewPortContext.hpp"
#include "PostProcessingSystem.h"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Lightning/LightStructs.hpp"
#include "Application/Utils/ApplicationUtils.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "RayTracing/RayTracer.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhr.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/Renderer/Renderers/UserInterfaceRenderer.hpp"
#include "Vulkan/Renderer/Renderers/ForwardRenderer.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VEnvLightGenerator/VEnvLightGenerator.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"
#include "Vulkan/Utils/VEffect/VRayTracingEffect.hpp"
#include "Vulkan/VulkanCore/Pipeline/VRayTracingPipeline.hpp"
#include "imgui.h"
#include <vulkan/vulkan_enums.hpp>


namespace Renderer {
RenderingSystem::RenderingSystem(const VulkanCore::VulkanInstance&    instance,
                                 const VulkanCore::VDevice&           device,
                                 VulkanUtils::VRayTracingDataManager& rayTracingDataManager,
                                 VulkanUtils::VUniformBufferManager&  uniformBufferManager,
                                 ApplicationCore::EffectsLibrary&     effectsLybrary,
                                 VulkanCore::VDescriptorLayoutCache&  descLayoutCache,
                                 VEditor::UIContext&                  uiContext)
    : m_device(device)
    , m_uniformBufferManager(uniformBufferManager)
    , m_renderContext()
    , m_uiContext(uiContext)
    , m_descLayoutCache(descLayoutCache)
    , m_transferSemapohore(device.GetTransferOpsManager().GetTransferSemaphore())
    , m_effectsLibrary(&effectsLybrary)
    , m_rayTracingDataManager(rayTracingDataManager)

{

    //---------------------------------------------------------------------------------------------------------------------------
    // Swap chain creation
    //---------------------------------------------------------------------------------------------------------------------------
    m_swapChain = std::make_unique<VulkanCore::VSwapChain>(m_device, instance);

    //------------------------------------------------------------------------------------------------------------------------
    // CREATE SYNCHRONIZATION PRIMITIVES and Command buffers
    //------------------------------------------------------------------------------------------------------------------------
    m_renderingTimeLine.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_imageAvailableSemaphores.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_renderingCommandBuffers.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_renderingCommandPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Graphics);
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_renderingTimeLine[i]        = std::make_unique<VulkanCore::VTimelineSemaphore>(m_device, 8);
        m_imageAvailableSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
        m_renderingCommandBuffers[i]  = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_renderingCommandPool);
    }
    m_ableToPresentSemaphore.resize(m_swapChain->GetImageCount());

    for(int i = 0; i < m_swapChain->GetImageCount(); i++)
        m_ableToPresentSemaphore[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);

    //----------------------------------------------------------------------------------------------------------------------------
    // Renderers creation
    //----------------------------------------------------------------------------------------------------------------------------

    m_forwardRenderer = std::make_unique<Renderer::ForwardRenderer>(m_device, &m_renderContext, effectsLybrary, descLayoutCache,
                                                                    GlobalVariables::RenderTargetResolutionWidth,
                                                                    GlobalVariables::RenderTargetResolutionHeight);

    m_postProcessingSystem = std::make_unique<Renderer::PostProcessingSystem>(m_device, effectsLybrary, uniformBufferManager,
                                                                              GlobalVariables::RenderTargetResolutionWidth,
                                                                              GlobalVariables::RenderTargetResolutionHeight);


    m_uiRenderer = std::make_unique<Renderer::UserInterfaceRenderer>(m_device, *m_swapChain, uiContext);

    m_uiContext.GetViewPortContext(ViewPortType::eMain).currentFrameInFlight = m_currentFrameIndex;

    m_envLightGenerator = std::make_unique<VulkanUtils::VEnvLightGenerator>(m_device, descLayoutCache);


    auto cam    = m_uiContext.GetClient().GetCamera();
    m_rayTracer = std::make_unique<RayTracer>(m_device, effectsLybrary, rayTracingDataManager, 1980, 1080);


    Utils::Logger::LogInfo("RenderingSystem initialized");
}

void RenderingSystem::Init()
{
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_uiContext.GetViewPortContext(ViewPortType::eMain).SetImage(m_postProcessingSystem->GetRenderedResult(i), i);
        //m_uiContext.GetViewPortContext(ViewPortType::eMain).SetImage(m_forwardRenderer->GetPositionBufferOutput().GetResolvedImage(), i);
        m_uiContext.GetViewPortContext(ViewPortType::eMainRayTracer).SetImage(m_postProcessingSystem->GetRenderedResult(i), i);
        m_uiContext.GetViewPortContext(ViewPortType::ePositionBuffer)
            .SetImage(m_forwardRenderer->GetPositionBufferOutput().GetResolvedImage(), i);
        m_uiContext.GetViewPortContext(ViewPortType::eShadowMap).SetImage(m_forwardRenderer->GetDenoisedVisibilityBuffer(), i);
        m_uiContext.GetViewPortContext(ViewPortType::ePositionBuffer)
            .SetImage(m_forwardRenderer->GetPositionBufferOutput().GetResolvedImage(), i);
        m_uiContext.GetViewPortContext(ViewPortType::eNormalBuffer).SetImage(m_forwardRenderer->GetNormalBufferOutput().GetPrimaryImage(), i);
    }
}

void RenderingSystem::Render(ApplicationCore::ApplicationState& applicationState)
{

    m_renderingTimeLine[m_currentFrameIndex]->CpuWaitIdle(8);


    m_sceneLightInfo = &applicationState.GetSceneLightInfo();
    //=================================================
    // GET SWAP IMAGE INDEX
    //=================================================
    auto imageIndex = VulkanUtils::SwapChainNextImageKHRWrapper(m_device, *m_swapChain, UINT64_MAX,
                                                                *m_imageAvailableSemaphores[m_currentFrameIndex], nullptr);

    if(applicationState.IsWindowResized())
    {
        imageIndex.first = vk::Result::eErrorOutOfDateKHR;
    }
    switch(imageIndex.first)
    {
        case vk::Result::eSuccess: {
            m_currentImageIndex = imageIndex.second;
            Utils::Logger::LogInfoVerboseRendering("Swap chain is successfuly retrieved");
            break;
        }
        case vk::Result::eErrorOutOfDateKHR: {

            m_swapChain->RecreateSwapChain();

            m_uiRenderer->GetRenderTarget().HandleSwapChainResize(*m_swapChain);

            m_renderingTimeLine[m_currentFrameIndex]->Reset();
            m_renderingTimeLine[m_currentFrameIndex]->CpuSignal(8);
            // to silent validation layers i will recreate the semaphore
            m_ableToPresentSemaphore[m_currentImageIndex]->Destroy();
            m_ableToPresentSemaphore[m_currentImageIndex] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);


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
    m_renderingCommandBuffers[m_currentFrameIndex]->Reset();
    m_frameCount++;


    if(applicationState.GetSceneUpdateFlags().resetAccumulation)
    {
        // reset the accumulation
        m_accumulatedFramesCount        = 0;
        m_renderContext.hasSceneChanged = true;
    }

    // ==== check if it is possible ot use env light
    if(m_isRayTracing)
    {
        applicationState.GetGlobalRenderingInfo().screenSize.x =
            m_rayTracer->GetRenderedImage(m_currentFrameIndex).GetImageInfo().width;
        applicationState.GetGlobalRenderingInfo().screenSize.y =
            m_rayTracer->GetRenderedImage(m_currentFrameIndex).GetImageInfo().height;
        // will cause to multiply by 0 thus clear the colour
        applicationState.GetGlobalRenderingInfo().numberOfFrames = m_accumulatedFramesCount;
    }
    else
    {
        applicationState.GetGlobalRenderingInfo().numberOfFrames = m_frameCount;
    }

    //=====================================================================
    // IMPORTANT: this sends all data accumulated over the frame to the GPU
    applicationState.GetGlobalRenderingInfo().isRayTracing = static_cast<int>(m_isRayTracing);
    m_uniformBufferManager.Update(m_currentFrameIndex, applicationState, m_renderContext.GetAllDrawCall());

    m_device.GetTransferOpsManager().UpdateGPUWaitCPU();

    //=================================================
    // sort the draw calls based on the state chagnes
    std::sort(m_renderContext.drawCalls.begin(), m_renderContext.drawCalls.end(),
              [](std::pair<unsigned long, VulkanStructs::VDrawCallData>& lhs,
                 std::pair<unsigned long, VulkanStructs::VDrawCallData>& rhs) { return lhs.first < rhs.first; });

    //=================================================
    // generate new IBL maps if new one was selected
    if(m_sceneLightInfo->environmentLight != nullptr)
        if(m_sceneLightInfo->environmentLight->hdrImage->IsAvailable())
            m_envLightGenerator->Generate(m_currentFrameIndex, m_sceneLightInfo->environmentLight->hdrImage->GetHandle(),
                                          *m_renderingTimeLine[m_currentFrameIndex]);
    m_renderContext.hdrCubeMap    = m_envLightGenerator->GetCubeMapRaw();
    m_renderContext.irradianceMap = m_envLightGenerator->GetIrradianceMapRaw();
    m_renderContext.prefilterMap  = m_envLightGenerator->GetPrefilterMapRaw();
    m_renderContext.brdfMap       = m_envLightGenerator->GetBRDFLutRaw();
    m_renderContext.dummyCubeMap  = m_envLightGenerator->GetDummyCubeMapRaw();
    m_renderContext.deltaTime     = ImGui::GetIO().DeltaTime;

    //==============================================================
    // Update descriptor writes
    if(m_frameCount > 2)
    {
        m_postProcessingSystem->Update(m_currentFrameIndex, m_postProcessingContext);
        m_effectsLibrary->UpdatePerFrameWrites(*m_forwardRenderer, m_rayTracingDataManager, &m_renderContext,
                                               m_postProcessingContext, m_uniformBufferManager);
    }

    //============================================================
    // start recording command buffer that will render the scene
    m_renderingCommandBuffers[m_currentFrameIndex]->BeginRecording();


    //==================================================
    // Fill in different contexts with parameters
    m_postProcessingContext.toneMappingParameters = &m_uniformBufferManager.GetApplicationState()->GetToneMappingParameters();
    m_postProcessingContext.lensFlareParameters = &m_uniformBufferManager.GetApplicationState()->GetLensFlareParameters();
    m_postProcessingContext.luminanceHistrogramParameters =
        &m_uniformBufferManager.GetApplicationState()->GetLuminanceHistogramParameters();
    m_postProcessingContext.luminanceAverageParameters =
        &m_uniformBufferManager.GetApplicationState()->GetLuminanceAverageParameters();
    m_postProcessingContext.deltaTime = ImGui::GetIO().DeltaTime;

    //===================================================
    // ACTUAL RENDERING IS TRIGGERED HERE
    if(!m_uiContext.m_isRayTracing)
    {
        // render scene
        m_forwardRenderer->Render(m_currentFrameIndex, *m_renderingCommandBuffers[m_currentFrameIndex],
                                  m_uniformBufferManager, &m_renderContext);

        m_postProcessingContext.sceneRender = m_forwardRenderer->GetForwardRendererResult();
        m_postProcessingContext.shadowMap   = &m_forwardRenderer->GetShadowMapOutput().GetPrimaryImage();
        m_postProcessingContext.toneMappingParameters->isRayTracing = false;
    }
    else
    {
        // path trace the scene
        m_rayTracer->TraceRays(*m_renderingCommandBuffers[m_currentFrameIndex], m_uniformBufferManager, m_currentFrameIndex);
        m_accumulatedFramesCount++;

        m_postProcessingContext.sceneRender = &m_rayTracer->GetRenderedImage(m_currentFrameIndex);
        m_postProcessingContext.toneMappingParameters->isRayTracing = true;
    }

    //========================================
    // Post processing
    m_postProcessingSystem->Render(m_currentFrameIndex, *m_renderingCommandBuffers[m_currentFrameIndex], m_postProcessingContext);

    //==========================================
    // UI Rendering
    m_uiRenderer->Render(m_currentFrameIndex, m_currentImageIndex, *m_renderingCommandBuffers[m_currentFrameIndex]);

    m_renderingCommandBuffers[m_currentFrameIndex]->EndRecording();

    //=====================================================
    // SUBMIT RECORDED COMMAND BUFFER
    //=====================================================
    vk::SubmitInfo submitInfo;

    const std::vector<vk::Semaphore> waitSemaphores   = {m_transferSemapohore.GetSemaphore(),
                                                         m_imageAvailableSemaphores[m_currentFrameIndex]->GetSyncPrimitive()};
    const std::vector<vk::Semaphore> signalSemaphores = {m_renderingTimeLine[m_currentFrameIndex]->GetSemaphore(),
                                                         m_ableToPresentSemaphore[m_currentImageIndex]->GetSyncPrimitive()};

    std::vector<vk::PipelineStageFlags> waitStages = {
        vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eColorAttachmentOutput,  // Render wait stage
        vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eColorAttachmentOutput,  // Render wait stage

        // Transfer wait stage
    };

    m_renderingTimeLine[m_currentFrameIndex]->SetWaitAndSignal(0, 8);  //
    //m_transferSemapohore.SetWaitAndSignal(2, 4);

    const std::vector<uint64_t> waitValues = {2, /*transfer- wait*/ 4 /*able to present - binary*/};
    const std::vector<uint64_t> signalVlaues = {m_renderingTimeLine[m_currentFrameIndex]->GetCurrentSignalValue() /*rendering signal*/,
                                                7 /*able to present - binary*/};

    vk::TimelineSemaphoreSubmitInfo timelineinfo;
    timelineinfo.waitSemaphoreValueCount = waitValues.size();
    timelineinfo.pWaitSemaphoreValues    = waitValues.data();

    timelineinfo.signalSemaphoreValueCount = signalVlaues.size();
    timelineinfo.pSignalSemaphoreValues    = signalVlaues.data();

    submitInfo.pNext              = &timelineinfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &m_renderingCommandBuffers[m_currentFrameIndex]->GetCommandBuffer();

    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores    = signalSemaphores.data();

    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores    = waitSemaphores.data();

    submitInfo.pWaitDstStageMask = waitStages.data();

    auto result = m_device.GetGraphicsQueue().submit(1, &submitInfo, nullptr);
    assert(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR);

    m_uiRenderer->Present(m_currentImageIndex, *m_renderingTimeLine[m_currentFrameIndex],
                          m_ableToPresentSemaphore[m_currentImageIndex]->GetSyncPrimitive());

    m_currentFrameIndex = (m_currentFrameIndex + 1) % GlobalVariables::MAX_FRAMES_IN_FLIGHT;

    m_renderContext.hasSceneChanged = false;
}

void RenderingSystem::Update()
{
    m_renderContext.ResetAllDrawCalls();
    m_uiContext.m_isRayTracing = m_isRayTracing;
}

void RenderingSystem::Destroy()
{
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_imageAvailableSemaphores[i]->Destroy();
        m_renderingTimeLine[i]->Destroy();
    }
    for(int i = 0; i < m_swapChain->GetImageCount(); i++)
    {
        m_ableToPresentSemaphore[i]->Destroy();
    }
    m_postProcessingSystem->Destroy();
    m_forwardRenderer->Destroy();
    m_uiRenderer->Destroy();
    m_swapChain->Destroy();
    m_envLightGenerator->Destroy();
    m_rayTracer->Destroy();
    m_renderingCommandPool->Destroy();
}


}  // namespace Renderer