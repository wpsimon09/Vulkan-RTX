//
// Created by wpsimon09 on 13/06/25.
//

#include "PostProcessingSystem.h"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

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

    //==================================
    // Get the tone mapping effect
    //==================================
    m_toneMappingEffect = effectsLibrary.GetEffect(ApplicationCore::EEffectType::ToneMappingPass);

    //====================================
    // Create tone mapping result image
    //===================================
    VulkanCore::VImage2CreateInfo toneMapOutputCi;
    toneMapOutputCi.height              = height;
    toneMapOutputCi.width               = width;
    toneMapOutputCi.imageAllocationName = "Final render";
    toneMapOutputCi.samples             = vk::SampleCountFlagBits::e1;
    toneMapOutputCi.imageUsage          = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
    toneMapOutputCi.layout              = vk::ImageLayout::eShaderReadOnlyOptimal;
    toneMapOutputCi.format              = vk::Format::eR8G8B8A8Srgb;

    for (int i = 0; i<GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {

      m_toneMapResult[i] = std::make_unique<VulkanCore::VImage2>(device, toneMapOutputCi);
      VulkanUtils::RecordImageTransitionLayoutCommand(*m_toneMapResult[i], vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eUndefined, m_device.GetTransferOpsManager().GetCommandBuffer());

    }

    Utils::Logger::LogInfo("Post processing system created");
}
}  // namespace Renderer