//
// Created by wpsimon09 on 21/12/24.
//

#include "ForwardRenderer.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"

#include <memory>
#include <sys/wait.h>
#include <Vulkan/Utils/VIimageTransitionCommands.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "RenderPass/DenoisePass.hpp"
#include "RenderPass/GBufferPass.hpp"
#include "RenderPass/LightPass.hpp"
#include "RenderPass/PostProcessing.hpp"
#include "RenderPass/VisibilityBufferPass.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/VisibilityBufferPass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/LightPass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/PostProcessing.hpp"


namespace Renderer {
ForwardRenderer::ForwardRenderer(const VulkanCore::VDevice&          device,
                                 VulkanUtils::RenderContext*         renderContext,
                                 ApplicationCore::EffectsLibrary&    effectsLibrary,
                                 VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                                 int                                 width,
                                 int                                 height)
    : m_device(device)
    , m_renderContextPtr(renderContext)

{
    Utils::Logger::LogInfo("Creating scene renderer");

    m_width  = width;
    m_height = height;

    //===========================================
    //===== CREATE RENDER TARGETS
    //===========================================


    //==================
    // Fog pass output
    Renderer::RenderTarget2CreatInfo fogPassOutputCI{
        width,
        height,
        false,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eNone,
    };

    m_fogPassOutput = std::make_unique<Renderer::RenderTarget2>(m_device, fogPassOutputCI);

    //==================================================================================================
    // New render pass system
    m_visibilityBufferPass  = std::make_unique<Renderer::VisibilityBufferPass>(device, effectsLibrary, width, height);
    m_gBufferPass           = std::make_unique<Renderer::GBufferPass>(device, effectsLibrary, width, height);
    m_visibilityDenoisePass = std::make_unique<Renderer::BilateralFilterPass>(
        device, effectsLibrary,
        m_visibilityBufferPass->GetPrimaryResult(EVisibilityBufferAttachments::VisibilityBuffer), width, height);
    m_forwardRenderPass = std::make_unique<Renderer::ForwardRender>(device, effectsLibrary, width, height);
    m_fogPass           = std::make_unique<Renderer::FogPass>(device, effectsLibrary, width, height);

    m_renderContextPtr->normalMap        = &m_gBufferPass->GetResolvedResult(EGBufferAttachments::Normal);
    m_renderContextPtr->positionMap      = &m_gBufferPass->GetResolvedResult(EGBufferAttachments::Position);
    m_renderContextPtr->depthBuffer      = &m_gBufferPass->GetDepthAttachment();
    m_renderContextPtr->visibilityBuffer = &m_visibilityDenoisePass->GetPrimaryResult();
    m_renderContextPtr->lightPassOutput  = &m_forwardRenderPass->GetResolvedResult();

    Utils::Logger::LogSuccess("Forward renderer created !");
}
void ForwardRenderer::Init(int                                  frameIndex,
                           VulkanUtils::VUniformBufferManager&  uniformBufferManager,
                           VulkanUtils::VRayTracingDataManager& rayTracingDataManager,
                           VulkanUtils::RenderContext*          renderContext)
{
    m_visibilityBufferPass->Init(frameIndex, uniformBufferManager, renderContext);
    m_gBufferPass->Init(frameIndex, uniformBufferManager, renderContext);
    m_visibilityDenoisePass->Init(frameIndex, uniformBufferManager, renderContext);
    m_forwardRenderPass->Init(frameIndex, uniformBufferManager, renderContext);
    m_fogPass->Init(frameIndex, uniformBufferManager, renderContext);
}

void ForwardRenderer::Update(int                                   currentFrame,
                             VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                             VulkanUtils::VRayTracingDataManager&  rayTracingDataManager,
                             VulkanUtils::RenderContext*           renderContext,
                             VulkanStructs::PostProcessingContext* postProcessingContext)
{
    m_visibilityBufferPass->Update(currentFrame, uniformBufferManager, renderContext, postProcessingContext);
    m_visibilityDenoisePass->Update(currentFrame, uniformBufferManager, renderContext, postProcessingContext);
    m_forwardRenderPass->Update(currentFrame, uniformBufferManager, renderContext, postProcessingContext);
    m_fogPass->Update(currentFrame, uniformBufferManager, renderContext, postProcessingContext);
}

void ForwardRenderer::Render(int                                       currentFrameIndex,
                             VulkanCore::VCommandBuffer&               cmdBuffer,
                             const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                             VulkanUtils::RenderContext*               renderContext)
{

    m_renderContextPtr = renderContext;
    //=====================================================
    // RECORD COMMAND BUFFER
    //=====================================================
    assert(cmdBuffer.GetIsRecording() && "Command buffer is not in recording state !");

    // descriptor set 0 is allways the samme

    //============================
    // generates depth buffer
    DepthPrePass(currentFrameIndex, cmdBuffer, uniformBufferManager);

    //===========================
    // generates shadow mapp in  screen space
    ShadowMapPass(currentFrameIndex, cmdBuffer, uniformBufferManager);

    //===========================
    // denoise the shadow pass
    DenoiseVisibility(currentFrameIndex, cmdBuffer, uniformBufferManager);

    //============================
    // uses forward renderer to render the scene
    DrawScene(currentFrameIndex, cmdBuffer, uniformBufferManager);

    //==================================
    // render the fog if it is in scene
    if(m_renderContextPtr->fogDrawCall)
    {
        PostProcessingFogPass(currentFrameIndex, cmdBuffer, uniformBufferManager);
    }


    m_frameCount++;
}
VulkanCore::VImage2* ForwardRenderer::GetForwardRendererResult() const
{
    return m_forwardRendererOutput;
}

Renderer::RenderTarget2& ForwardRenderer::GetDepthPrePassOutput() const
{
    return m_gBufferPass->GetDepthAttachment();
}
Renderer::RenderTarget2& ForwardRenderer::GetPositionBufferOutput() const
{
    return m_gBufferPass->GetRenderTarget(EGBufferAttachments::Position);
}
Renderer::RenderTarget2& ForwardRenderer::GetShadowMapOutput() const
{
    return m_visibilityBufferPass->GetRenderTarget();
}
Renderer::RenderTarget2& ForwardRenderer::GetLightPassOutput() const
{
    return m_forwardRenderPass->GetRenderTarget();
}

Renderer::RenderTarget2& ForwardRenderer::GetNormalBufferOutput() const
{
    return m_gBufferPass->GetRenderTarget(EGBufferAttachments::Normal);
}

VulkanCore::VImage2& ForwardRenderer::GetDenoisedVisibilityBuffer() const
{
    return m_visibilityDenoisePass->GetPrimaryResult(EBilateralFilterAttachments::Result);
}

void ForwardRenderer::DepthPrePass(int                                       currentFrameIndex,
                                   VulkanCore::VCommandBuffer&               cmdBuffer,
                                   const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    int drawCallCount = 0;
    m_gBufferPass->Render(currentFrameIndex, cmdBuffer, m_renderContextPtr);
}

void ForwardRenderer::ShadowMapPass(int                                       currentFrameIndex,
                                    VulkanCore::VCommandBuffer&               cmdBuffer,
                                    const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{

    assert(cmdBuffer.GetIsRecording() && "Command buffer is not recording ! ");
    m_visibilityBufferPass->Render(currentFrameIndex, cmdBuffer, m_renderContextPtr);
}

//==================================================================
// De noiser pass, applies bilateral filter to the visibility buffer

void ForwardRenderer::DenoiseVisibility(int                                       currentFrameIndex,
                                        VulkanCore::VCommandBuffer&               cmdBuffer,
                                        const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    m_visibilityDenoisePass->Render(currentFrameIndex, cmdBuffer, m_renderContextPtr);
}


void ForwardRenderer::DrawScene(int                                       currentFrameIndex,
                                VulkanCore::VCommandBuffer&               cmdBuffer,
                                const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{

    assert(cmdBuffer.GetIsRecording() && "Command buffer is not in recording state !");
    m_forwardRenderPass->Render(currentFrameIndex, cmdBuffer, m_renderContextPtr);
    m_forwardRendererOutput = &m_forwardRenderPass->GetResolvedResult();
}
void ForwardRenderer::PostProcessingFogPass(int                                       currentFrameIndex,
                                            VulkanCore::VCommandBuffer&               cmdBuffer,
                                            const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    // this might not be the best thing to do but for now it should suffice
    m_fogPass->Render(currentFrameIndex, cmdBuffer, m_renderContextPtr);
    m_forwardRendererOutput         = &m_fogPass->GetPrimaryResult();
    m_renderContextPtr->fogDrawCall = nullptr;
}


void ForwardRenderer::Destroy()
{
    //m_renderTargets->Destroy();
    m_fogPassOutput->Destroy();
    m_visibilityBufferPass->Destroy();
    m_gBufferPass->Destroy();
    m_visibilityDenoisePass->Destroy();
    //m_shadowMap->Destroy();
}
}  // namespace Renderer
