//
// Created by wpsimon09 on 28/09/2025.
//

#include "AtmospherePass.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include <memory>

namespace Renderer {
AtmospherePass::AtmospherePass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height)
    : RenderPass(device, width, height)
{
    m_transmitanceLutEffect =
        effectsLibrary.GetEffect<VulkanUtils::VComputeEffect>(ApplicationCore::EEffectType::AtmosphereTransmitanceLUT);

    Renderer::RenderTarget2CreatInfo transmitanceLutCi{256,
                                                       64,
                                                       false,
                                                       false,
                                                       vk::Format::eR16G16B16A16Sfloat,
                                                       vk::ImageLayout::eGeneral,
                                                       vk::ResolveModeFlagBits::eNone,
                                                       true};
    m_transmittanceLut = std::make_unique<RenderTarget2>(device, transmitanceLutCi);
}

void AtmospherePass::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    m_transmitanceLutEffect->SetNumWrites(0, 1, 0);
    m_transmitanceLutEffect->WriteImage(currentFrameIndex, 0, 0, m_transmittanceLut->GetPrimaryImage().GetDescriptorImageInfo());
    m_transmitanceLutEffect->ApplyWrites(currentFrameIndex);
}

void AtmospherePass::Update(int                                   currentFrame,
                            VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                            VulkanUtils::RenderContext*           renderContext,
                            VulkanStructs::PostProcessingContext* postProcessingContext)
{
}

void AtmospherePass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
}

void AtmospherePass::Destroy() {}

}  // namespace Renderer