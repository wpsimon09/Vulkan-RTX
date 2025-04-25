//
// Created by wpsimon09 on 20/04/25.
//

#include "RayTracer.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VRayTracingEffect.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Vulkan/Utils/VResrouceGroup/VResourceGroupManager.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingStructs.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace Renderer {
RayTracer::RayTracer(const VulkanCore::VDevice&           device,
                     VulkanUtils::VResourceGroupManager&  resourceGroupManager,
                     VulkanUtils::VRayTracingDataManager& rtxDataManager,
                     int                                  width,
                     int                                  height)
    : m_device(device)
    , m_rtxDataManager(rtxDataManager)
    , m_resourceGroupManager(resourceGroupManager)
{
    m_resultImage.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {

        VulkanCore::VImage2CreateInfo imageCI;
        imageCI.width  = width;
        imageCI.height = height;
        imageCI.imageUsage =
            vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
        imageCI.layout = vk::ImageLayout::eGeneral;
        imageCI.format = vk::Format::eR16G16B16A16Sfloat;

        m_resultImage[i] = std::make_unique<VulkanCore::VImage2>(device, imageCI);

        auto& cmdBuffer = m_device.GetTransferOpsManager().GetCommandBuffer();

        VulkanUtils::RecordImageTransitionLayoutCommand(*m_resultImage[i], vk::ImageLayout::eGeneral,
                                                        vk::ImageLayout::eUndefined, cmdBuffer);
    }


    VulkanCore::RTX::RTXShaderPaths rtxShaderPaths;
    rtxShaderPaths.rayGenPath = "Shaders/Compiled/SimpleRayTracing.rgen.spv";
    rtxShaderPaths.missPath   = "Shaders/Compiled/SimpleRayTracing.miss.spv";
    rtxShaderPaths.rayHitPath = "Shaders/Compiled/SimpleRayTracing.chit.spv";
    auto rayTracingHitGroup   = std::make_unique<VulkanUtils::VRayTracingEffect>(
        device, rtxShaderPaths, "Hit group highlight",
        m_resourceGroupManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::RayTracing));

    m_rtxEffect = std::move(rayTracingHitGroup);

    m_rtxEffect->BuildEffect();
}
void RayTracer::TraceRays(const VulkanCore::VCommandBuffer&         cmdBuffer,
                          const VulkanCore::VTimelineSemaphore&     renderingSemaphore,
                          const VulkanUtils::VUniformBufferManager& unifromBufferManager,
                          int                                       currentFrame)
{
    auto& descriptor = std::get<VulkanUtils::RayTracingDescriptorSet>(m_rtxEffect->GetResrouceGroupStructVariant());
    descriptor.buffer1 = unifromBufferManager.GetGlobalBufferDescriptorInfo()[currentFrame];
    descriptor.buffer2 = unifromBufferManager.GetLightBufferDescriptorInfo()[currentFrame];

    descriptor.tlas = m_rtxDataManager.GetTlasWriteInfo();
    descriptor.storage2D_1 = m_resultImage[currentFrame]->GetDescriptorImageInfo();
}


void RayTracer::ProcessResize(int newWidth, int newHeight) {}

void RayTracer::Destroy()
{
    for(auto& i : m_resultImage)
    {
        i->Destroy();
    }
}
}  // namespace Renderer