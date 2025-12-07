//
// Created by wpsimon09 on 13/06/25.
//

#include "PostProcessingSystem.h"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "RenderPass/PostProcessing.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Renderer/Renderers/RenderPass/RenderPass.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include <cstdint>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

namespace Renderer {

PostProcessingSystem::PostProcessingSystem(const VulkanCore::VDevice&          device,
                                           ApplicationCore::EffectsLibrary&    effectsLibrary,
                                           VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                           int                                 width,
                                           int                                 height)
    : m_device(device)
    , m_uniformBufferManager(uniformBufferManager)
    , m_width(width)
    , m_height(height)
{
    Utils::Logger::LogInfo("Creating post processing system");


    m_toneMappingPass = std::make_unique<Renderer::ToneMappingPass>(device, effectsLibrary, width, height);
    m_lensFlarePass   = std::make_unique<Renderer::LensFlarePass>(device, effectsLibrary, width, height);
    m_bloomPass       = std::make_unique<Renderer::BloomPass>(device, effectsLibrary, width, height);
    m_compositionPass = std::make_unique<Renderer::CompositePass>(device, effectsLibrary, width, height);

    m_finalRender = &m_toneMappingPass->GetPrimaryResult(EToneMappingAttachments::LDR);

    Utils::Logger::LogInfo("Post processing system created");
}

void PostProcessingSystem::Render(int frameIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext)
{

    if(!postProcessingContext.isRayTracing && m_uniformBufferManager.GetApplicationState()->m_composite)
    {
        Composite(frameIndex, commandBuffer, postProcessingContext);
    }
    if(postProcessingContext.bloomEffect)
    {
        Bloom(frameIndex, commandBuffer, postProcessingContext);
    }
    ToneMapping(frameIndex, commandBuffer, postProcessingContext);
    if(postProcessingContext.lensFlareEffect)
    {
        LensFlare(frameIndex, commandBuffer, postProcessingContext);
    }
}

void PostProcessingSystem::Update(int                                   frameIndex,
                                  VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                                  VulkanStructs::PostProcessingContext& postProcessingCotext)
{
    if(!postProcessingCotext.isRayTracing && m_uniformBufferManager.GetApplicationState()->m_composite)
    {
        m_compositionPass->Update(frameIndex, uniformBufferManager, nullptr, &postProcessingCotext);
        postProcessingCotext.sceneRender = &m_compositionPass->GetPrimaryResult(0);
    }

    if(postProcessingCotext.bloomEffect)
    {
        m_bloomPass->Update(frameIndex, uniformBufferManager, nullptr, &postProcessingCotext);
        postProcessingCotext.sceneRender = &m_bloomPass->GetRenderTarget(EBloomAttachments::BloomOutput).GetPrimaryImage();
    }

    m_toneMappingPass->Update(frameIndex, uniformBufferManager, nullptr, &postProcessingCotext);

    if(postProcessingCotext.lensFlareEffect)
    {
        postProcessingCotext.sceneRender = &m_toneMappingPass->GetPrimaryResult(EToneMappingAttachments::LDR);
        m_lensFlarePass->Update(frameIndex, uniformBufferManager, nullptr, &postProcessingCotext);
    }
}


VulkanCore::VImage2& PostProcessingSystem::GetRenderedResult(int frameIndex)
{
    return *m_finalRender;
}
void PostProcessingSystem::Init(int                                   frameIndex,
                                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                                VulkanUtils::RenderContext*           renderContext,
                                VulkanStructs::PostProcessingContext* postProcessingContext)
{
    m_bloomPass->Init(frameIndex, uniformBufferManager, renderContext);
    m_toneMappingPass->Init(frameIndex, uniformBufferManager, renderContext);
    m_lensFlarePass->Init(frameIndex, uniformBufferManager, renderContext);
    m_compositionPass->Init(frameIndex, uniformBufferManager, renderContext);
}


void PostProcessingSystem::ToneMapping(int                                   currentIndex,
                                       VulkanCore::VCommandBuffer&           commandBuffer,
                                       VulkanStructs::PostProcessingContext& postProcessingContext)
{
    //=================================================================
    // Transition tone mapping output to the output attachment optimal
    m_toneMappingPass->Render(currentIndex, commandBuffer, nullptr);
    m_finalRender = &m_toneMappingPass->GetPrimaryResult(EToneMappingAttachments::LDR);
}


void PostProcessingSystem::LensFlare(int                                   currentIndex,
                                     VulkanCore::VCommandBuffer&           commandBuffer,
                                     VulkanStructs::PostProcessingContext& postProcessingContext)
{
    m_lensFlarePass->Render(currentIndex, commandBuffer, nullptr);
    m_finalRender = &m_lensFlarePass->GetPrimaryResult(ELensFlareAttachments::LensFlareMain);
}


void PostProcessingSystem::Bloom(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext)
{
    m_bloomPass->Render(currentIndex, commandBuffer, nullptr);
}

void PostProcessingSystem::Composite(int                                   currentIndex,
                                     VulkanCore::VCommandBuffer&           commandBuffer,
                                     VulkanStructs::PostProcessingContext& postProcessingContext)
{
    m_compositionPass->Render(currentIndex, commandBuffer, nullptr);
    postProcessingContext.sceneRender = &m_compositionPass->GetPrimaryResult(0);
}


void PostProcessingSystem::Destroy()
{
    m_toneMappingPass->Destroy();
    m_lensFlarePass->Destroy();
    m_bloomPass->Destroy();
    m_compositionPass->Destroy();
}


}  // namespace Renderer