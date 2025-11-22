//
// Created by wpsimon09 on 16/08/2025.
//

#include "VisibilityBufferPass.hpp"
#include "RenderPass.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace Renderer {
VisibilityBufferPass::VisibilityBufferPass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height)
    : Renderer::RenderPass(device, width, height)
    , m_aoOcclusionParameters{}
{
    //=================================================
    // create the effect
    m_rayTracedShadowEffect = effectLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::RTShadowPass);

    //===================================================
    // create render target
    Renderer::RenderTarget2CreatInfo shadowMapCI{
        width / 2, height / 2, false, false, vk::Format::eR16Sfloat, vk::ImageLayout::eGeneral, vk::ResolveModeFlagBits::eNone, true, "visibility buffer attachment"};

    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, shadowMapCI));

    shadowMapCI.imageDebugName = "Ambient occlusion map";
    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, shadowMapCI));
}
void VisibilityBufferPass::Init(int frameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{

    m_rayTracedShadowEffect->SetNumWrites(3, 5, 1);

    m_rayTracedShadowEffect->WriteBuffer(frameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[frameIndex]);

    m_rayTracedShadowEffect->WriteBuffer(frameIndex, 0, 1, uniformBufferManager.GetLightBufferDescriptorInfo()[frameIndex]);

    m_rayTracedShadowEffect->WriteAccelerationStrucutre(frameIndex, 0, 2, renderContext->tlas);

    m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 3,
                                        renderContext->positionMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerDepth));

    m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 4,
                                        MathUtils::LookUpTables.BlueNoise1024->GetHandle()->GetDescriptorImageInfo(
                                            VulkanCore::VSamplers::Sampler2D));

    m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 5,
                                        renderContext->normalMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerDepth));

    m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 6,
                                        GetPrimaryAttachemntDescriptorInfo(EVisibilityBufferAttachments::ShadowMap));

    m_rayTracedShadowEffect->ApplyWrites(frameIndex);
}

void VisibilityBufferPass::Update(int                                   currentFrame,
                                  VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                                  VulkanUtils::RenderContext*           renderContext,
                                  VulkanStructs::PostProcessingContext* postProcessingContext)
{
    m_rayTracedShadowEffect->SetNumWrites(0, 0, 1);
    m_rayTracedShadowEffect->WriteAccelerationStrucutre(currentFrame, 0, 2, renderContext->tlas);
    m_rayTracedShadowEffect->ApplyWrites(currentFrame);

    m_aoOcclusionParameters = uniformBufferManager.GetApplicationState()->GetAoOcclusionParameters();
}

void VisibilityBufferPass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{

    VulkanUtils::PlacePipelineBarrier(cmdBuffer, vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                      vk::PipelineStageFlagBits::eFragmentShader);

    //=========================================================================
    // Transition shadow map from shader read only optimal to general
    m_renderTargets[EVisibilityBufferAttachments::ShadowMap]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eAttachmentOptimal,
                                                                                    vk::ImageLayout::eShaderReadOnlyOptimal,
                                                                                    VulkanUtils::VImage_SampledRead_To_General);


    auto& cmdB = cmdBuffer.GetCommandBuffer();

    m_rayTracedShadowEffect->BindPipeline(cmdB);
    m_rayTracedShadowEffect->BindDescriptorSet(cmdB, currentFrame, 0);

    //dispatch
    float width, height;
    width  = m_renderTargets[EVisibilityBufferAttachments::ShadowMap]->GetWidth();
    height = m_renderTargets[EVisibilityBufferAttachments::ShadowMap]->GetHeight();
    cmdB.dispatch(width / 8, height / 8, 1);

    m_renderTargets[EVisibilityBufferAttachments::ShadowMap]->TransitionAttachments(
        cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eAttachmentOptimal,
        VulkanUtils::VImage_SampledRead_To_General.Switch());
}

void VisibilityBufferPass::Destroy()
{
    RenderPass::Destroy();
    m_rayTracedShadowEffect->Destroy();
}

}  // namespace Renderer