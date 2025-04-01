//
// Created by wpsimon09 on 01/04/25.
//

#include "VEnvLightGenerator.hpp"

#include "Vulkan/VulkanCore/VImage/VImage2.hpp"


VulkanUtils::VEnvLightGenerator::VEnvLightGenerator(const VulkanCore::VDevice& device): m_device(device),
    m_envMapGenerationSemphore(device), m_envGenerationTransferOpsManager(device)
{
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


    vk::RenderingAttachmentInfo brdfAttachmentCI;;
    brdfAttachmentCI.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    brdfAttachmentCI.imageView = m_brdfLut->GetImageView();
    brdfAttachmentCI.loadOp = vk::AttachmentLoadOp::eClear;
    brdfAttachmentCI.resolveMode = vk::ResolveModeFlagBits::eNone;
    brdfAttachmentCI.storeOp = vk::AttachmentStoreOp::eStore;
    brdfAttachmentCI.clearValue.color.setFloat32({0.2f, 0.2f, 0.2f, 1.f});





}
