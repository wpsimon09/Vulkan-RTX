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
#include "Vulkan/Renderer/Renderers/RenderPass/VisibilityBufferPass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/GBufferPass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/DenoisePass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/LightPass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/PostProcessing.hpp"


#include "imgui.h"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore2.hpp"

#include <vulkan/vulkan_enums.hpp>

/**
 * DISCLAIMER:
 * this class needs major refactor together with submitting transfer operations, but in the current time i have no idea
 * how to go with this, so i am gona keep it as it is, because it works and does to job for now
 */
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
    m_frameTimeLine.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_imageAvailableSemaphores.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_renderingCommandBuffers.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_renderingCommandPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Graphics);
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_frameTimeLine[i] = std::make_unique<VulkanCore::VTimelineSemaphore2>(m_device, EFrameStages::NumStages);
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

    m_uiContext.GetViewPortContext(ViewPortType::eMain).currentFrameInFlight = m_currentFrameInFlight;

    m_envLightGenerator = std::make_unique<VulkanUtils::VEnvLightGenerator>(m_device, descLayoutCache);

    m_rayTracer = std::make_unique<RayTracer>(m_device, effectsLybrary, rayTracingDataManager, 1980, 1080);

    Utils::Logger::LogInfo("RenderingSystem initialized");
}

void RenderingSystem::Init()
{
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {

        m_uiContext.GetViewPortContext(ViewPortType::eMain).SetImage(m_postProcessingSystem->GetRenderedResult(0), i);
        m_uiContext.GetViewPortContext(ViewPortType::eMainRayTracer).SetImage(m_postProcessingSystem->GetRenderedResult(i), i);
        m_uiContext.GetViewPortContext(ViewPortType::ePositionBuffer)
            .SetImage(m_forwardRenderer->GetPositionBufferOutput().GetResolvedImage(), i);
        m_uiContext.GetViewPortContext(ViewPortType::eShadowMap).SetImage(m_forwardRenderer->GetDenoisedVisibilityBuffer(), i);
        m_uiContext.GetViewPortContext(ViewPortType::ePositionBuffer)
            .SetImage(m_forwardRenderer->GetPositionBufferOutput().GetResolvedImage(), i);
        m_uiContext.GetViewPortContext(ViewPortType::eNormalBuffer).SetImage(m_forwardRenderer->GetNormalBufferOutput().GetResolvedImage(), i);

        // Init forward rendere
        m_forwardRenderer->Init(i, m_uniformBufferManager, m_rayTracingDataManager, &m_renderContext);

        //init post processing system
        m_postProcessingSystem->Init(i, m_uniformBufferManager, &m_renderContext, &m_postProcessingContext);
    }
}
void RenderingSystem::WaitForSignalToProcede(int stage)
{
    if(m_frameCount >= GlobalVariables::MAX_FRAMES_IN_FLIGHT)
    {
        m_frameTimeLine[m_currentFrameInFlight]->CpuWaitIdle(stage);

        if (stage == EFrameStages::SafeToBeginDrawCallRecording) {
            m_frameTimeLine[m_currentFrameInFlight]->ProcedeToNextFrame();
        }
    }
}


void RenderingSystem::Update(ApplicationCore::ApplicationState& applicationState)
{
    m_sceneLightInfo = &applicationState.GetSceneLightInfo();

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
            m_rayTracer->GetRenderedImage(m_currentFrameInFlight).GetImageInfo().width;
        applicationState.GetGlobalRenderingInfo().screenSize.y =
            m_rayTracer->GetRenderedImage(m_currentFrameInFlight).GetImageInfo().height;
        // will cause to multiply by 0 thus clear the colour
        applicationState.GetGlobalRenderingInfo().numberOfFrames = m_accumulatedFramesCount;
    }
    else
    {
        applicationState.GetGlobalRenderingInfo().numberOfFrames = m_frameCount;
    }

    //==================================================
    // Pass parameters to the post processing context
    m_postProcessingContext.toneMappingParameters         = &applicationState.GetToneMappingParameters();
    m_postProcessingContext.lensFlareParameters           = &applicationState.GetLensFlareParameters();
    m_postProcessingContext.luminanceHistrogramParameters = &applicationState.GetLuminanceHistogramParameters();
    m_postProcessingContext.luminanceAverageParameters    = &applicationState.GetLuminanceAverageParameters();
    m_postProcessingContext.deltaTime                     = ImGui::GetIO().DeltaTime;

    m_renderContext.tlas          = m_rayTracingDataManager.GetTLAS();

    //=====================================================================
    // IMPORTANT: this sends all data accumulated over the frame to the GPU
    applicationState.GetGlobalRenderingInfo().isRayTracing = static_cast<int>(m_isRayTracing);
    m_uniformBufferManager.Update(m_currentFrameInFlight, applicationState, m_renderContext.GetAllDrawCall());
}


void RenderingSystem::Render(ApplicationCore::ApplicationState& applicationState)
{
    //============================================================
    // start recording command buffer that will render the scene


    if(m_frameCount >= GlobalVariables::MAX_FRAMES_IN_FLIGHT)
    {
        m_frameTimeLine[m_currentFrameInFlight]->CpuWaitIdle(EFrameStages::SafeToBeginDrawCallRecording);
        m_frameTimeLine[m_currentFrameInFlight]->ProcedeToNextFrame();
    }

    m_acquiredImage = VulkanUtils::SwapChainNextImageKHRWrapper(m_device, *m_swapChain, UINT64_MAX,
                                                                *m_imageAvailableSemaphores[m_currentSwapChainImage], nullptr);

    auto swapChainImageIndex = m_acquiredImage;

    if(applicationState.IsWindowResized())
    {
        swapChainImageIndex.first = vk::Result::eErrorOutOfDateKHR;
    }
    switch(swapChainImageIndex.first)
    {
    case vk::Result::eSuccess: {
        m_currentSwapChainImage = m_acquiredImage.second;
        Utils::Logger::LogInfoVerboseRendering("Swap chain is successfuly retrieved");
        break;
    }
    case vk::Result::eErrorOutOfDateKHR: {

        m_swapChain->RecreateSwapChain();
        m_uiRenderer->HandleSwapChainResize(*m_swapChain);

        m_frameTimeLine[m_currentFrameInFlight]->CpuSignal(EFrameStages::SafeToBeginDrawCallRecording);
        // to silent validation layers i will recreate the semaphore
        m_ableToPresentSemaphore[m_acquiredImage.second]->Destroy();
        m_ableToPresentSemaphore[m_acquiredImage.second] =
            std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);

        m_frameCount++;
        m_device.CurrentFrame = m_frameCount;

        return;
    }
    case vk::Result::eSuboptimalKHR: {
        m_currentSwapChainImage = swapChainImageIndex.second;
        break;
        //m_swapChain->RecreateSwapChain();
        //return;
    };
    default:
        break;
    }

    m_renderingCommandBuffers[m_currentFrameInFlight]->Reset();
    m_renderingCommandBuffers[m_currentFrameInFlight]->BeginRecording();


      //=================================================
    // sort the draw calls based on the state chagnes
    std::sort(m_renderContext.drawCalls.begin(), m_renderContext.drawCalls.end(),
              [](std::pair<unsigned long, VulkanStructs::VDrawCallData>& lhs,
                 std::pair<unsigned long, VulkanStructs::VDrawCallData>& rhs) { return lhs.first < rhs.first; });

    //=================================================
    // generate new IBL maps if new one was selected
    if(m_sceneLightInfo->environmentLight != nullptr)
        if(m_sceneLightInfo->environmentLight->hdrImage->IsAvailable())
            m_envLightGenerator->Generate(m_currentFrameInFlight, m_sceneLightInfo->environmentLight->hdrImage->GetHandle(),
                                          *m_frameTimeLine[m_currentFrameInFlight]);
    //====================================================================
    // pass necessary data to the rendering context
    m_renderContext.hdrCubeMap    = m_envLightGenerator->GetCubeMapRaw();
    m_renderContext.irradianceMap = m_envLightGenerator->GetIrradianceMapRaw();
    m_renderContext.prefilterMap  = m_envLightGenerator->GetPrefilterMapRaw();
    m_renderContext.brdfMap       = m_envLightGenerator->GetBRDFLutRaw();
    m_renderContext.dummyCubeMap  = m_envLightGenerator->GetDummyCubeMapRaw();
    m_renderContext.deltaTime     = ImGui::GetIO().DeltaTime;



    //===================================================
    // ACTUAL RENDERING IS TRIGGERED HERE
    if(!m_uiContext.m_isRayTracing)
    {
        //===========================================================
        // update render passes
        m_forwardRenderer->WriteDescriptorSets(m_currentFrameInFlight, m_uniformBufferManager, m_rayTracingDataManager, &m_renderContext,
                                  &m_postProcessingContext);
        // render scene
        m_forwardRenderer->Render(m_currentFrameInFlight, *m_renderingCommandBuffers[m_currentFrameInFlight],
                                  m_uniformBufferManager, &m_renderContext);

        m_postProcessingContext.sceneRender = m_forwardRenderer->GetForwardRendererResult();
        m_postProcessingContext.shadowMap   = &m_forwardRenderer->GetShadowMapOutput().GetPrimaryImage();
        m_postProcessingContext.toneMappingParameters->isRayTracing = false;
    }
    else
    {
        // path trace the scene
        m_rayTracer->TraceRays(*m_renderingCommandBuffers[m_currentFrameInFlight], m_uniformBufferManager, m_currentFrameInFlight);
        m_accumulatedFramesCount++;

        m_postProcessingContext.sceneRender = &m_rayTracer->GetRenderedImage(m_currentFrameInFlight);
        m_postProcessingContext.toneMappingParameters->isRayTracing = true;
    }


    //========================================
    // Post processing
    m_postProcessingSystem->WriteDescriptorSets(m_currentFrameInFlight, m_uniformBufferManager, m_postProcessingContext);
    m_postProcessingSystem->Render(m_currentFrameInFlight, *m_renderingCommandBuffers[m_currentFrameInFlight], m_postProcessingContext);

    m_uiContext.GetViewPortContext(ViewPortType::eMain).OverwriteImage(m_postProcessingSystem->GetRenderedResult(m_currentFrameInFlight), m_currentFrameInFlight);
    //==========================================
    // UI Rendering
    m_uiRenderer->Render(m_currentFrameInFlight, m_currentSwapChainImage, *m_renderingCommandBuffers[m_currentFrameInFlight]);
}


void RenderingSystem::SubmitFrame()
{

    //=====================================================
    // SUBMIT RECORDED COMMAND BUFFER
    //=====================================================
    m_device.GetTransferOpsManager().UpdateGPU(*m_frameTimeLine[m_currentFrameInFlight]);

    vk::SemaphoreSubmitInfo imageAvailableSubmitInfo = {
        m_imageAvailableSemaphores[m_currentFrameInFlight]->GetSyncPrimitive(), {}, vk::PipelineStageFlagBits2::eAllCommands, {}, {}};
    /*
      * Before we can render, we have to wait for 2 things:
      * 1. all transfer operations have to be done
      * 2. image to render into must be acquired from the swap chain
      */
    std::vector<vk::SemaphoreSubmitInfo> waitSemaphres = {
        // wait until transfer is finished
        m_frameTimeLine[m_currentFrameInFlight]->GetSemaphoreWaitSubmitInfo(
            EFrameStages::TransferFinish, vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eRayTracingShaderKHR),
        // wait until image to present is awailable
        imageAvailableSubmitInfo};

    /*
    * After the rendering is done we need to proceede to the next frame by signaling m_renderingTimelineSemaphore
    * and informing presentation engine and UI renderer that the presentation can happen since rendering is done
    */
    vk::SemaphoreSubmitInfo ableToPresentSubmitInfo = {
        m_ableToPresentSemaphore[m_acquiredImage.second]->GetSyncPrimitive(), {}, vk::PipelineStageFlagBits2::eAllCommands};

    std::vector<vk::SemaphoreSubmitInfo> signalSemaphores = {
        // rendering timeline will signal 8 which means that new frame can start exectuing
        m_frameTimeLine[m_currentFrameInFlight]->GetSemaphoreSignalSubmitInfo(EFrameStages::SafeToBeginDrawCallRecording,
                                                                           vk::PipelineStageFlagBits2::eAllCommands),
        // able to present semaphore should be singaled once rendering is finished
        ableToPresentSubmitInfo};

    m_renderingCommandBuffers[m_currentFrameInFlight]->EndAndFlush2(m_device.GetGraphicsQueue(), signalSemaphores, waitSemaphres);

    m_uiRenderer->Present(m_acquiredImage.second, m_ableToPresentSemaphore[m_acquiredImage.second]->GetSyncPrimitive());

    m_currentFrameInFlight = (m_currentFrameInFlight + 1) % GlobalVariables::MAX_FRAMES_IN_FLIGHT;

    m_device.CurrentFrame++;
    m_device.CurrentFrameInFlight   = m_currentFrameInFlight;
    m_renderContext.hasSceneChanged = false;


    m_frameCount++;


    //=======================================
    // reset everything
    m_renderContext.ResetAllDrawCalls();
    m_uiContext.m_isRayTracing = m_isRayTracing;
}
void RenderingSystem::Destroy()
{
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_imageAvailableSemaphores[i]->Destroy();
        m_frameTimeLine[i]->Destroy();
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
VulkanCore::VTimelineSemaphore2& RenderingSystem::GetTimelineSemaphore()
{
    return *m_frameTimeLine[m_currentFrameInFlight];
}


}  // namespace Renderer