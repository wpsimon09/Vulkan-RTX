//
// Created by wpsimon09 on 16/08/2025.
//

#include "VisibilityBufferPass.hpp"
#include "RenderPass.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"

namespace Renderer {
VisibilityBufferPass::VisibilityBufferPass(const VulkanCore::VDevice& device, VulkanCore::VDescriptorLayoutCache& descLayoutCache, int width, int height)
    : Renderer::RenderPass(device, width, height)
{
    //=================================================
    // create the effect
    m_rayTracedShadowEffect = std::make_unique<VulkanUtils::VRasterEffect>(device, "Ray traced shadow map effect",
                                                                           "Shaders/Compiled/RTShadowPass.vert.spv",
                                                                           "Shaders/Compiled/RTShadowPass.frag.spv",
                                                                           descLayoutCache, EShaderBindingGroup::ShadowRT);
    m_rayTracedShadowEffect->SetDisableDepthTest()
        .DisableStencil()
        .SetCullNone()
        .SetNullVertexBinding()
        .SetColourOutputFormat(vk::Format::eR16G16B16A16Sfloat)
        .SetPiplineNoMultiSampling();

    m_rayTracedShadowEffect->BuildEffect();

    //===================================================
    // create render target

    //==================
    // Shadow map
    Renderer::RenderTarget2CreatInfo shadowMapCI{
        width,
        height,
        false,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eNone,
    };

    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, shadowMapCI));

}
void VisibilityBufferPass::Init(int frameIndex, VulkanUtils::VUniformBufferManager&  uniformBufferManager,
                           VulkanUtils::VRayTracingDataManager& rayTracingDataManager,
                           VulkanUtils::RenderContext*          renderContext)
{

        m_rayTracedShadowEffect->SetNumWrites(3, 4, 1);

        m_rayTracedShadowEffect->WriteBuffer(frameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[frameIndex]);

        m_rayTracedShadowEffect->WriteBuffer(frameIndex, 0, 1, uniformBufferManager.GetLightBufferDescriptorInfo()[frameIndex]);

        m_rayTracedShadowEffect->WriteAccelerationStrucutre(frameIndex, 0, 2, rayTracingDataManager.GetTLAS());

        m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 3, renderContext->normalMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerDepth));

        m_rayTracedShadowEffect->WriteImage(
            frameIndex, 0, 4, MathUtils::LookUpTables.BlueNoise1024->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

        m_rayTracedShadowEffect->WriteImage(frameIndex, 0, 5, renderContext->positionMap->GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerDepth));


        m_rayTracedShadowEffect->ApplyWrites(frameIndex);

}

void VisibilityBufferPass::Update(int                                   currentFrame,
                             VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                             VulkanUtils::VRayTracingDataManager&  rayTracingDataManager,
                             VulkanUtils::RenderContext*           renderContext,
                             VulkanStructs::PostProcessingContext* postProcessingContext)
{
}

void VisibilityBufferPass::Render(int currentFrame, VulkanUtils::RenderContext* renderContext) {

}

}  // namespace