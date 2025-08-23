//
// Created by wpsimon09 on 13/06/25.
//

#include "PostProcessingSystem.h"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "RenderPass/PostProcessing.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VIimageTransitionCommands.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include <cstdint>
#include <memory>
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

    m_finalRender = &m_toneMappingPass->GetPrimaryResult(EToneMappingAttachments::LDR);

    Utils::Logger::LogInfo("Post processing system created");
}

void PostProcessingSystem::Render(int frameIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext)
{

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
    m_toneMappingPass->Init(frameIndex, uniformBufferManager, renderContext);
    m_lensFlarePass->Init(frameIndex, uniformBufferManager, renderContext);
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

void PostProcessingSystem::Destroy()
{
    m_toneMappingPass->Destroy();
    m_lensFlarePass->Destroy();
}


}  // namespace Renderer