//
// Created by wpsimon09 on 20/04/25.
//

#include "RayTracer.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace Renderer {
RayTracer::RayTracer(const VulkanCore::VDevice& device, VulkanUtils::VRayTracingDataManager& rtxDataManager, int width, int height)
    : m_device(device)
    , m_rtxDataManager(rtxDataManager)
{
    m_resultImage.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {

        VulkanCore::VImage2CreateInfo imageCI;
        imageCI.width  = width;
        imageCI.height = height;
        imageCI.imageUsage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc;
        imageCI.layout = vk::ImageLayout::eGeneral;

        m_resultImage[i] = std::make_unique<VulkanCore::VImage2>(device, imageCI);

        auto& cmdBuffer = m_device.GetTransferOpsManager().GetCommandBuffer();

        VulkanUtils::RecordImageTransitionLayoutCommand(*m_resultImage[i], vk::ImageLayout::eGeneral, vk::ImageLayout::eUndefined, cmdBuffer);
    }
}
void RayTracer::TraceRays(const VulkanCore::VCommandBuffer&     cmdBuffer,
                          const VulkanCore::VTimelineSemaphore& renderingSemaphore,
                          VulkanUtils::VUniformBufferManager&   unifromBufferManager,
                          int                                   currentFrame)
{
}

void RayTracer::ProcessResize(int newWidth, int newHeight) {}

void RayTracer::Destroy() {
  for (auto& i:m_resultImage) {
    i->Destroy();
  }
}
} // Renderer