//
// Created by wpsimon09 on 01/04/25.
//

#include "VEnvLightGenerator.hpp"

#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"


VulkanUtils::VEnvLightGenerator::VEnvLightGenerator(const VulkanCore::VDevice& device, VulkanUtils::VPushDescriptorManager& pushDescriptorManager): m_device(device),
    m_envMapGenerationSemphore(device), m_envGenerationTransferOpsManager(device), m_pushDescriptorManager(pushDescriptorManager)
{
    m_graphicsCmdPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Graphics);
    m_transferCmdPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Transfer);

    m_graphicsCmdBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_graphicsCmdPool);
    m_transferCmdBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_transferCmdPool);

    GenerateBRDFLut();
}

void VulkanUtils::VEnvLightGenerator::GenerateBRDFLut()
{
    //============p===========================================
    // CREATE INFO FOR BRDF LOOK UP IMAGE
    //=======================================================
    VulkanCore::VImage2CreateInfo brdfCI; // CI -create info
    brdfCI.channels = 2;
    brdfCI.format = vk::Format::eR16G16Sfloat;
    brdfCI.width = 512;
    brdfCI.height = 512;
    m_brdfLut = std::make_unique<VulkanCore::VImage2>(m_device, brdfCI);

    m_transferCmdBuffer->BeginRecording();

    RecordImageTransitionLayoutCommand(*m_brdfLut, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eUndefined, *m_transferCmdBuffer);

    std::vector<vk::PipelineStageFlags> waitStages = {
        vk::PipelineStageFlagBits::eTopOfPipe,
    };

    m_transferCmdBuffer->EndAndFlush(
        m_device.GetTransferQueue(),
        m_envMapGenerationSemphore.GetSemaphore(),
        m_envMapGenerationSemphore.GetSemaphoreSubmitInfo(0, 2),
        waitStages.data()
        );

    vk::RenderingAttachmentInfo brdfAttachmentCI;;
    brdfAttachmentCI.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    brdfAttachmentCI.imageView = m_brdfLut->GetImageView();
    brdfAttachmentCI.loadOp = vk::AttachmentLoadOp::eClear;
    brdfAttachmentCI.resolveMode = vk::ResolveModeFlagBits::eNone;
    brdfAttachmentCI.storeOp = vk::AttachmentStoreOp::eStore;
    brdfAttachmentCI.clearValue.color.setFloat32({0.0f, 0.0f, 0.0f, 1.f});

    //Create Effect that generates BRDF
    VEffect brdfEffect(m_device, "BRDF Effect",
        "Shaders/Compiled/BRDFLut.vert.spv",
        "Shaders/Compiled/BRDFLut.frag.spv",
        m_pushDescriptorManager.GetPushDescriptor(EDescriptorLayoutStruct::Basic)
        );
    brdfEffect.SetColourOutputFormat(brdfCI.format)
    .DisableStencil()
    .SetDisableDepthTest()
    .SetCullNone();

    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent = vk::Extent2D(brdfCI.width, brdfCI.height);
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &brdfAttachmentCI;;
    renderingInfo.pDepthAttachment = nullptr;
    renderingInfo.pStencilAttachment = nullptr;

    m_graphicsCmdBuffer->BeginRecording();
    auto& cmdBuffer = m_graphicsCmdBuffer->GetCommandBuffer();
    cmdBuffer.beginRendering(&renderingInfo);

    vk::Viewport viewport {0, 0, (float)brdfCI.width, (float)brdfCI.height, 0.0f, 1.0f};
    cmdBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D  scissors {{0, 0}, {(uint32_t)brdfCI.width, (uint32_t)brdfCI.height} };
    cmdBuffer.setScissor(0, 1, &scissors);

    brdfEffect.BindPipeline(cmdBuffer);
    cmdBuffer.draw(3, 1, 0, 0);





}
