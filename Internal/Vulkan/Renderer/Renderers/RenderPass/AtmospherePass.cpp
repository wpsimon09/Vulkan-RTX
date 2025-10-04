//
// Created by wpsimon09 on 28/09/2025.
//

#include "AtmospherePass.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include <memory>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

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
    m_renderTargets.push_back(std::make_unique<RenderTarget2>(device, transmitanceLutCi));
}

void AtmospherePass::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    renderContext->transmitanceLut = &m_renderTargets[EAtmosphereAttachments::TransmitanceLUT]->GetPrimaryImage();

    m_transmitanceLutEffect->SetNumWrites(0, 1, 0);
    m_transmitanceLutEffect->WriteImage(
        currentFrameIndex, 0, 0,
        m_renderTargets[EAtmosphereAttachments::TransmitanceLUT]->GetPrimaryImage().GetDescriptorImageInfo());
    m_transmitanceLutEffect->ApplyWrites(currentFrameIndex);
}

void AtmospherePass::Update(int                                   currentFrame,
                            VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                            VulkanUtils::RenderContext*           renderContext,
                            VulkanStructs::PostProcessingContext* postProcessingContext)
{
    if(uniformBufferManager.GetApplicationState()->GetAtmosphereParameters())
        m_atmosphereParams = *uniformBufferManager.GetApplicationState()->GetAtmosphereParameters();
}

void AtmospherePass::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    std::cout << "Rendering atmospherer\n";
}

void AtmospherePass::Precompute(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{

    Utils::Logger::LogInfo("Precomputeing transmitance");
    m_transmitanceLutEffect->BindPipeline(cmdBuffer.GetCommandBuffer());
    m_transmitanceLutEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

    vk::PushConstantsInfo pcInfo;
    pcInfo.layout     = m_transmitanceLutEffect->GetPipelineLayout();
    pcInfo.offset     = 0;
    pcInfo.pValues    = &m_atmosphereParams;
    pcInfo.size       = sizeof(m_atmosphereParams);
    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

    m_transmitanceLutEffect->CmdPushConstant(cmdBuffer.GetCommandBuffer(), pcInfo);

    cmdBuffer.GetCommandBuffer().dispatch(m_renderTargets[EAtmosphereAttachments::TransmitanceLUT]->GetWidth() / 16,
                                          m_renderTargets[EAtmosphereAttachments::TransmitanceLUT]->GetHeight() / 16, 1);

    VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite,
                                                vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                                                vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderSampledRead};

    VulkanUtils::PlaceImageMemoryBarrier2(m_renderTargets[EAtmosphereAttachments::TransmitanceLUT]->GetPrimaryImage(),
                                          cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eGeneral, barrierPos);
}

void AtmospherePass::Destroy() {}

}  // namespace Renderer