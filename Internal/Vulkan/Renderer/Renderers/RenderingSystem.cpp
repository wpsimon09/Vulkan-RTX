//
// Created by wpsimon09 on 21/12/24.
//

#include "RenderingSystem.hpp"

#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Lightning/LightStructs.hpp"
#include "Application/Utils/ApplicationUtils.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/VResrouceGroup/VResourceGroupManager.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "RayTracing/RayTracer.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhr.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/Renderer/Renderers/UserInterfaceRenderer.hpp"
#include "Vulkan/Renderer/Renderers/SceneRenderer.hpp"
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


namespace Renderer {
RenderingSystem::RenderingSystem(const VulkanCore::VulkanInstance&         instance,
                                 const VulkanCore::VDevice&                device,
                                 const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                 VulkanUtils::VResourceGroupManager&       pushDescriptorManager,
                                 VEditor::UIContext&                       uiContext)
    : m_device(device)
    , m_uniformBufferManager(uniformBufferManager)
    , m_resrouceGroupManager(pushDescriptorManager)
    , m_renderContext()
    , m_uiContext(uiContext)
    , m_transferSemapohore(device.GetTransferOpsManager().GetTransferSemaphore())
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
    m_ableToPresentSemaphore.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_renderingCommandPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Graphics);
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_ableToPresentSemaphore[i]   = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
        m_renderingTimeLine[i]        = std::make_unique<VulkanCore::VTimelineSemaphore>(m_device, 8);
        m_imageAvailableSemaphores[i] = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
        m_renderingCommandBuffers[i]  = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_renderingCommandPool);
    }

    //----------------------------------------------------------------------------------------------------------------------------
    // Renderers creation
    //----------------------------------------------------------------------------------------------------------------------------

    m_sceneRenderer = std::make_unique<Renderer::SceneRenderer>(m_device, m_resrouceGroupManager,
                                                                GlobalVariables::RenderTargetResolutionWidth,
                                                                GlobalVariables::RenderTargetResolutionHeight);

    m_uiRenderer = std::make_unique<Renderer::UserInterfaceRenderer>(m_device, *m_swapChain, uiContext);

    m_uiContext.GetViewPortContext(ViewPortType::eMain).currentFrameInFlight = m_currentFrameIndex;

    m_envLightGenerator = std::make_unique<VulkanUtils::VEnvLightGenerator>(m_device, pushDescriptorManager);


    //----------------------------------------------------------------------------------------------------------------------------
    // Ray tracing initialization
    //----------------------------------------------------------------------------------------------------------------------------
    m_rayTracingDataManager = std::make_unique<VulkanUtils::VRayTracingDataManager>(m_device);

    auto cam    = m_uiContext.GetClient().GetCamera();
    m_rayTracer = std::make_unique<RayTracer>(m_device, m_resrouceGroupManager, *m_rayTracingDataManager, 1980, 1080);

    Utils::Logger::LogInfo("RenderingSystem initialized");
}

void RenderingSystem::Init()
{
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_uiContext.GetViewPortContext(ViewPortType::eMain).SetImage(m_sceneRenderer->GetRenderedImage(i), i);
        //m_uiContext.GetViewPortContext(ViewPortType::eMain).SetImage(m_envLightGenerator->GetBRDFLut(), i);
        m_uiContext.GetViewPortContext(ViewPortType::eMainRayTracer).SetImage(m_rayTracer->GetRenderedImage(i), i);
    }
}

void RenderingSystem::Render(LightStructs::SceneLightInfo& sceneLightInfo,
                             ApplicationCore::SceneData&   sceneData,
                             GlobalUniform&                globalUniformUpdateInfo,
                             SceneUpdateFlags&             sceneUpdateFlags)
{
    m_renderingTimeLine[m_currentFrameIndex]->CpuWaitIdle(8);

    m_sceneLightInfo = &sceneLightInfo;

    //=================================================
    // GET SWAP IMAGE INDEX
    //=================================================
    auto imageIndex = VulkanUtils::SwapChainNextImageKHRWrapper(m_device, *m_swapChain, UINT64_MAX,
                                                                *m_imageAvailableSemaphores[m_currentFrameIndex], nullptr);

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

    if(sceneUpdateFlags.resetAccumulation)
    {
        // reset the accumulation
        m_accumulatedFramesCount = 0;
    }

    // ==== check if it is possible ot use env light
    if(m_isRayTracing)
    {
        globalUniformUpdateInfo.screenSize.x = m_rayTracer->GetRenderedImage(m_currentFrameIndex).GetImageInfo().width;
        globalUniformUpdateInfo.screenSize.y = m_rayTracer->GetRenderedImage(m_currentFrameIndex).GetImageInfo().height;
        // will cause to multiply by 0 thus clear the colour
        globalUniformUpdateInfo.numberOfFrames = m_accumulatedFramesCount;
    }
    else
    {
        globalUniformUpdateInfo.numberOfFrames = m_frameCount;
    }

    m_uniformBufferManager.UpdatePerFrameUniformData(m_currentFrameIndex, globalUniformUpdateInfo);
    m_uniformBufferManager.UpdateLightUniformData(m_currentFrameIndex, sceneLightInfo);
    m_uniformBufferManager.UpdatePerObjectUniformData(m_currentFrameIndex, m_renderContext.GetAllDrawCall());
    m_uniformBufferManager.UpdateSceneDataInfo(m_currentFrameIndex, sceneData);

    m_device.GetTransferOpsManager().UpdateGPU();
    m_transferSemapohore.CpuWaitIdle(2);
    m_transferSemapohore.Reset();

    std::sort(m_renderContext.drawCalls.begin(), m_renderContext.drawCalls.end(),
              [](std::pair<unsigned long, VulkanStructs::DrawCallData>& lhs,
                 std::pair<unsigned long, VulkanStructs::DrawCallData>& rhs) { return lhs.first < rhs.first; });

    // generate new IBL maps if new one was selected
    if(sceneLightInfo.environmentLight != nullptr)
        if(sceneLightInfo.environmentLight->hdrImage->IsAvailable())
            m_envLightGenerator->Generate(sceneLightInfo.environmentLight->hdrImage->GetHandle(),
                                          *m_renderingTimeLine[m_currentFrameIndex]);
    m_renderContext.hdrCubeMap    = m_envLightGenerator->GetCubeMapRaw();
    m_renderContext.irradianceMap = m_envLightGenerator->GetIrradianceMapRaw();
    m_renderContext.prefilterMap  = m_envLightGenerator->GetPrefilterMapRaw();
    m_renderContext.brdfMap       = m_envLightGenerator->GetBRDFLutRaw();
    m_renderContext.dummyCubeMap  = m_envLightGenerator->GetDummyCubeMapRaw();


    //============================================================
    // start recording command buffer that will render the scene
    m_renderingCommandBuffers[m_currentFrameIndex]->BeginRecording();

    if(!m_uiContext.m_isRayTracing)
    {
        // render scene
        m_sceneRenderer->Render(m_currentFrameIndex, *m_renderingCommandBuffers[m_currentFrameIndex],
                                m_uniformBufferManager, &m_renderContext);
    }
    else
    {
        // path trace the scene
        m_rayTracer->TraceRays(*m_renderingCommandBuffers[m_currentFrameIndex], sceneUpdateFlags,
                               m_uniformBufferManager, m_currentFrameIndex);
        m_accumulatedFramesCount++;
    }

    // render UI to the swap chain image
    m_uiRenderer->Render(m_currentFrameIndex, m_currentImageIndex, *m_renderingCommandBuffers[m_currentFrameIndex]);

    m_renderingCommandBuffers[m_currentFrameIndex]->EndRecording();

    //=====================================================
    // SUBMIT RECORDED COMMAND BUFFER
    //=====================================================
    vk::SubmitInfo submitInfo;

    const std::vector<vk::Semaphore> waitSemaphores   = {m_transferSemapohore.GetSemaphore(),
                                                         m_imageAvailableSemaphores[m_currentFrameIndex]->GetSyncPrimitive()};
    const std::vector<vk::Semaphore> signalSemaphores = {m_renderingTimeLine[m_currentFrameIndex]->GetSemaphore(),
                                                         m_ableToPresentSemaphore[m_currentFrameIndex]->GetSyncPrimitive()};

    std::vector<vk::PipelineStageFlags> waitStages = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput,  // Render wait stage
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eNone
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
                          m_ableToPresentSemaphore[m_currentFrameIndex]->GetSyncPrimitive());

    m_currentFrameIndex = (m_currentFrameIndex + 1) % GlobalVariables::MAX_FRAMES_IN_FLIGHT;
}

void RenderingSystem::Update()
{
    m_renderContext.ResetAllDrawCalls();
    m_uiContext.m_isRayTracing = m_isRayTracing;
}

void RenderingSystem::Destroy()
{
    m_rayTracingDataManager->Destroy();
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_imageAvailableSemaphores[i]->Destroy();
        m_ableToPresentSemaphore[i]->Destroy();
        m_renderingTimeLine[i]->Destroy();
    }
    m_sceneRenderer->Destroy();
    m_uiRenderer->Destroy();
    m_swapChain->Destroy();
    m_envLightGenerator->Destroy();
    m_rayTracer->Destroy();
    m_renderingCommandPool->Destroy();

}


}  // namespace Renderer