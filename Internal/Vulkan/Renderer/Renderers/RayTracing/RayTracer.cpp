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
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

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
    m_accumulationResultImage.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
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

        imageCI.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
        imageCI.layout     = vk::ImageLayout::eColorAttachmentOptimal;
        imageCI.format     = vk::Format::eR16G16B16A16Sfloat;
        imageCI.isStorage = false;


        m_accumulationResultImage[i] = std::make_unique<VulkanCore::VImage2>(device, imageCI);

        VulkanUtils::RecordImageTransitionLayoutCommand(*m_accumulationResultImage[i], vk::ImageLayout::eShaderReadOnlyOptimal,
                                                        vk::ImageLayout::eUndefined, cmdBuffer);
    }


    VulkanCore::RTX::RTXShaderPaths rtxShaderPaths;
    rtxShaderPaths.rayGenPath     = "Shaders/Compiled/SimpleRayTracing.rgen.spv";
    rtxShaderPaths.missPath       = "Shaders/Compiled/SimpleRayTracing.miss.spv";
    rtxShaderPaths.missShadowPath = "Shaders/Compiled/SimpleRayTracing.miss2.spv";
    rtxShaderPaths.rayHitPath     = "Shaders/Compiled/SimpleRayTracing.chit.spv";
    auto rayTracingHitGroup       = std::make_unique<VulkanUtils::VRayTracingEffect>(
        device, rtxShaderPaths, "Hit group highlight",
        m_resourceGroupManager.GetResourceGroup(VulkanUtils::EDescriptorLayoutStruct::RayTracing));

    m_rtxEffect = std::move(rayTracingHitGroup);
    m_rtxEffect->BuildEffect();

    m_accumulationEffect = std::make_unique<VulkanUtils::VRasterEffect>(
        m_device, "Accumulation of ray traced images effect", "Shaders/Compiled/RayTracingAccumultion.vert.spv",
        "Shaders/Compiled/RayTracingAccumultion.frag.spv",
        m_resourceGroupManager.GetResourceGroup(VulkanUtils::EDescriptorLayoutStruct::PostProcessing));

    m_accumulationEffect->SetColourOutputFormat(vk::Format::eR16G16B16A16Sfloat)
        .DisableStencil()
        .SetDisableDepthTest()
        .SetCullNone()
        .SetNullVertexBinding()
        .SetPiplineNoMultiSampling();
    m_accumulationEffect->BuildEffect();
}

void RayTracer::TraceRays(const VulkanCore::VCommandBuffer&         cmdBuffer,
                          const VulkanCore::VTimelineSemaphore&     renderingSemaphore,
                          const VulkanUtils::VUniformBufferManager& unifromBufferManager,
                          const ApplicationCore::SceneData&         sceneData,
                          int                                       currentFrame)
{
    assert(cmdBuffer.GetIsRecording() && "Command buffer is not recordgin !");

    int previousFrame;
    if (currentFrame == 1) {
        previousFrame = 0;
    }else if (currentFrame == 0) {
        previousFrame = 1;
    }

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
    descriptor.buffer4     = unifromBufferManager.GetSceneBufferDescriptorInfo(currentFrame);
    descriptor.texture2D_2 = m_resultImage[previousFrame]->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

    cmdB.pushDescriptorSetWithTemplateKHR(m_rtxEffect->GetUpdateTemplate(), m_rtxEffect->GetPipelineLayout(), 0,
                                          descriptor, m_device.DispatchLoader);

    auto& p = m_rtxEffect->GetRTXPipeline();
    cmdB.traceRaysKHR(p.m_rGenRegion, p.m_rMissRegion, p.m_rHitRegion, p.m_rCallRegion,
                      m_resultImage[currentFrame]->GetImageInfo().width,
                      m_resultImage[currentFrame]->GetImageInfo().height, 1, m_device.DispatchLoader);

    VulkanUtils::RecordImageTransitionLayoutCommand(*m_resultImage[currentFrame], vk::ImageLayout::eShaderReadOnlyOptimal,
                                                    vk::ImageLayout::eGeneral, cmdB);

    //======================================
    // Accumulate the the samples
    //======================================

    /**
    //  configure render pass and attachment stuff
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_accumulationResultImage[previousFrame], vk::ImageLayout::eShaderReadOnlyOptimal,
                                                    vk::ImageLayout::eColorAttachmentOptimal, cmdB);

    // configure render pass and attachment stuff
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_accumulationResultImage[currentFrame], vk::ImageLayout::eColorAttachmentOptimal,
                                                    vk::ImageLayout::eShaderReadOnlyOptimal, cmdB);

    vk::RenderingAttachmentInfo accumulationAttachment;
    accumulationAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    accumulationAttachment.imageView   = m_accumulationResultImage[currentFrame]->GetImageView();
    accumulationAttachment.loadOp      = vk::AttachmentLoadOp::eClear;
    accumulationAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
    accumulationAttachment.storeOp     = vk::AttachmentStoreOp::eStore;
    accumulationAttachment.clearValue.color.setFloat32({0.0f, 0.0f, 0.0f, 1.f});


    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset        = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent.width  = m_resultImage[currentFrame]->GetImageInfo().width;
    renderingInfo.renderArea.extent.height = m_resultImage[currentFrame]->GetImageInfo().height;
    renderingInfo.layerCount               = 1;
    renderingInfo.colorAttachmentCount     = 1;
    renderingInfo.pColorAttachments        = &accumulationAttachment;

    cmdB.beginRendering(&renderingInfo);

    vk::Viewport viewport{
        0, 0, (float)renderingInfo.renderArea.extent.width, (float)renderingInfo.renderArea.extent.height, 0.0f, 1.0f};
    cmdB.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{{0, 0}, {(uint32_t)renderingInfo.renderArea.extent.width, (uint32_t)renderingInfo.renderArea.extent.height}};
    cmdB.setScissor(0, 1, &scissors);
    cmdB.setStencilTestEnable(false);

    auto& descriptorAccumulation   = std::get<VulkanUtils::PostProcessingDescriptorSet>(m_accumulationEffect->GetResrouceGroupStructVariant());

    m_accumulationEffect->BindPipeline(cmdB);

    descriptorAccumulation.buffer1 = unifromBufferManager.GetGlobalBufferDescriptorInfo()[currentFrame];
    descriptorAccumulation.texture2D_1 = m_resultImage[previousFrame]->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
    descriptorAccumulation.texture2D_2 = m_accumulationResultImage[previousFrame]->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

    cmdB.pushDescriptorSetWithTemplateKHR(m_accumulationEffect->GetUpdateTemplate(), m_accumulationEffect->GetPipelineLayout(), 0,
                                          descriptorAccumulation, m_device.DispatchLoader);

    cmdB.draw(3,1,0,0);

    cmdB.endRendering();

    VulkanUtils::RecordImageTransitionLayoutCommand(*m_accumulationResultImage[currentFrame], vk::ImageLayout::eShaderReadOnlyOptimal,vk::ImageLayout::eColorAttachmentOptimal, cmdB);
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_accumulationResultImage[previousFrame], vk::ImageLayout::eColorAttachmentOptimal,vk::ImageLayout::eShaderReadOnlyOptimal, cmdB);
        */
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