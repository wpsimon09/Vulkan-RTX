//
// Created by wpsimon09 on 20/04/25.
//

#include "RayTracer.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VRayTracingEffect.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Vulkan/Utils/VResrouceGroup/VResourceGroupManager.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingStructs.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/VulkanCore/Pipeline/VRayTracingPipeline.hpp"

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
        imageCI.layout    = vk::ImageLayout::eGeneral;
        imageCI.format    = vk::Format::eR16G16B16A16Sfloat;
        imageCI.isStorage = true;

        m_resultImage[i] = std::make_unique<VulkanCore::VImage2>(device, imageCI);

        auto& cmdBuffer = m_device.GetTransferOpsManager().GetCommandBuffer();

        VulkanUtils::RecordImageTransitionLayoutCommand(*m_resultImage[i], vk::ImageLayout::eShaderReadOnlyOptimal,
                                                        vk::ImageLayout::eUndefined, cmdBuffer);
    }


    VulkanCore::RTX::RTXShaderPaths rtxShaderPaths;
    rtxShaderPaths.rayGenPath     = "Shaders/Compiled/SimpleRayTracing.rgen.spv";
    rtxShaderPaths.missPath       = "Shaders/Compiled/SimpleRayTracing.miss.spv";
    rtxShaderPaths.missShadowPath = "Shaders/Compiled/SimpleRayTracing.miss2.spv";
    rtxShaderPaths.rayHitPath     = "Shaders/Compiled/SimpleRayTracing.chit.spv";
    auto rayTracingHitGroup       = std::make_unique<VulkanUtils::VRayTracingEffect>(
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
    assert(cmdBuffer.GetIsRecording() && "Command buffer is not recordgin !");
    m_rtxEffect->BindPipeline(cmdBuffer.GetCommandBuffer());

    auto& cmdB = cmdBuffer.GetCommandBuffer();

    VulkanUtils::RecordImageTransitionLayoutCommand(*m_resultImage[currentFrame], vk::ImageLayout::eGeneral,
                                                    vk::ImageLayout::eShaderReadOnlyOptimal, cmdB);

    auto& descriptor   = std::get<VulkanUtils::RayTracingDescriptorSet>(m_rtxEffect->GetResrouceGroupStructVariant());
    descriptor.buffer1 = unifromBufferManager.GetGlobalBufferDescriptorInfo()[currentFrame];
    descriptor.buffer2 = unifromBufferManager.GetLightBufferDescriptorInfo()[currentFrame];
    descriptor.buffer3 = m_rtxDataManager.GetObjDescriptionBufferInfo();
    descriptor.tlas    = m_rtxDataManager.GetTLAS();
    descriptor.storage2D_1 = m_resultImage[currentFrame]->GetDescriptorImageInfo();

    cmdB.pushDescriptorSetWithTemplateKHR(m_rtxEffect->GetUpdateTemplate(), m_rtxEffect->GetPipelineLayout(), 0,
                                          descriptor, m_device.DispatchLoader);

    auto& p = m_rtxEffect->GetRTXPipeline();
    cmdB.traceRaysKHR(p.m_rGenRegion, p.m_rMissRegion, p.m_rHitRegion, p.m_rCallRegion,
                      m_resultImage[currentFrame]->GetImageInfo().width,
                      m_resultImage[currentFrame]->GetImageInfo().height, 1, m_device.DispatchLoader);

    VulkanUtils::RecordImageTransitionLayoutCommand(*m_resultImage[currentFrame], vk::ImageLayout::eShaderReadOnlyOptimal,
                                                    vk::ImageLayout::eGeneral, cmdB);
}


void                 RayTracer::ProcessResize(int newWidth, int newHeight) {}
VulkanCore::VImage2& RayTracer::GetRenderedImage(int currentFrameIndex)
{
    return *m_resultImage[currentFrameIndex];
}

void RayTracer::Destroy()
{
    m_rtxEffect->Destroy();
    for(auto& i : m_resultImage)
    {
        i->Destroy();
    }
}
}  // namespace Renderer