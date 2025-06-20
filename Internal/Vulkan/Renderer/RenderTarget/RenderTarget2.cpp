//
// Created by wpsimon09 on 20/06/25.
//

#include "RenderTarget2.h"

#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace Renderer {
RenderTarget2::RenderTarget2(const VulkanCore::VDevice& device, RenderTarget2CreatInfo& createInfo): m_device(device) {

  //==================================================
  // Creat main attachment image
  //==================================================
  VulkanCore::VImage2CreateInfo attachemtImageCI;
  attachemtImageCI.format     = createInfo.format;
  attachemtImageCI.height     = createInfo.heigh;
  attachemtImageCI.width      = createInfo.width;
  attachemtImageCI.samples = createInfo.multiSampled ? m_device.GetSampleCount() : vk::SampleCountFlagBits::e1;
  attachemtImageCI.mipLevels  = 1;
  attachemtImageCI.aspecFlags = createInfo.isDepth ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
  if (createInfo.isDepth) {
    attachemtImageCI.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
                                            | vk::ImageUsageFlagBits::eInputAttachment;
  }else {
    attachemtImageCI.imageUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
  }
  m_multisampledAttachment = std::make_unique<VulkanCore::VImage2>(m_device, attachemtImageCI);


  //===================================
  // transition to specified layout
  //===================================
  auto& cmdBuffer = m_device.GetTransferOpsManager().GetCommandBuffer();

  VulkanUtils::RecordImageTransitionLayoutCommand(*m_multisampledAttachment,
                                                createInfo.initialLayout, vk::ImageLayout::eUndefined,
                                                cmdBuffer.GetCommandBuffer());


  //=================================================
  // Create resolve target image
  //=================================================
  if (createInfo.multiSampled) {
    m_resolvedAttachment = std::make_unique<VulkanCore::VImage2>(m_device, attachemtImageCI);

    //===================================
    // transition to specified layout
    //===================================
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_multisampledAttachment,
                                                createInfo.initialLayout, vk::ImageLayout::eUndefined,
                                                cmdBuffer.GetCommandBuffer());
  }




}
} // Renderer