//
// Created by wpsimon09 on 06/10/24.
//

#include "VRenderPass.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"

VulkanCore::VRenderPass::VRenderPass(const VulkanCore::VDevice &device, const VulkanCore::VSwapChain &swapChain,const VulkanCore::VImage& depthBuffer):VObject(),m_device(device), m_swapChain(swapChain), m_depthBuffer(depthBuffer) {
    Utils::Logger::LogInfoVerboseOnly("Creating render pass...");
    CreateRenderPass();
}

void VulkanCore::VRenderPass::Destroy() {
    Utils::Logger::LogInfoVerboseOnly("Render pass destoryed");
    m_device.GetDevice().destroyRenderPass(m_renderPass);
}

void VulkanCore::VRenderPass::CreateRenderPass() {

    //------------------------
    // BASIC COLOUR ATTACHMENT
    //------------------------
    m_colourAttachmentDescription.format = m_swapChain.GetSurfaceFormatKHR().format;
    m_colourAttachmentDescription.samples = vk::SampleCountFlagBits::e1;
    m_colourAttachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
    m_colourAttachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
    m_colourAttachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    m_colourAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    m_colourAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
    m_colourAttachmentDescription.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    m_colourAttachmentRef.attachment = 0;
    m_colourAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    //-----------------
    // DEPTH ATTACHMENT
    //-----------------
    m_depthStencilAttachmentDescription.format = m_depthBuffer.GetFormat();
    m_depthStencilAttachmentDescription.samples = vk::SampleCountFlagBits::e1;
    m_depthStencilAttachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
    m_depthStencilAttachmentDescription.storeOp = vk::AttachmentStoreOp::eDontCare;
    m_depthStencilAttachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    m_depthStencilAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    m_depthStencilAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
    m_depthStencilAttachmentDescription.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    m_depthStencilAttachmentRef.attachment = 1;
    m_depthStencilAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;


    //----------------------------------------------------------------------------------------------------------------------------
    // RESOLVE ATTACHEMENT
    // it transition the image format from colour attachment to the format that is suitable to present the images on the screen
    //----------------------------------------------------------------------------------------------------------------------------
    /*
    m_resolveColourAttachmentDescription.format =   m_swapChain.GetSurfaceFormatKHR().format;
    m_resolveColourAttachmentDescription.samples = vk::SampleCountFlagBits::e1;
    m_resolveColourAttachmentDescription.loadOp = vk::AttachmentLoadOp::eDontCare;
    m_resolveColourAttachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
    m_resolveColourAttachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    m_resolveColourAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    m_resolveColourAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
    m_resolveColourAttachmentDescription.finalLayout = vk::ImageLayout::ePresentSrcKHR;
    */

    CreateMainSubPass();

    std::array<vk::AttachmentDescription, 2> attachments = {m_colourAttachmentDescription, m_depthStencilAttachmentDescription};
    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &m_subPass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &m_subPassDependency;

    m_renderPass = m_device.GetDevice().createRenderPass(renderPassInfo);
    assert(m_renderPass);
    Utils::Logger::LogSuccess("Render pass created");
}

void VulkanCore::VRenderPass::CreateMainSubPass() {
    m_subPass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    m_subPass.colorAttachmentCount = 1;
    m_subPass.pColorAttachments = &m_colourAttachmentRef;
    m_subPass.pDepthStencilAttachment = &m_depthStencilAttachmentRef;

    m_subPassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    m_subPassDependency.dstSubpass = 0;

    // at what stage this sub pass is happening and what memory we want to access
    m_subPassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    m_subPassDependency.srcAccessMask = vk::AccessFlagBits::eNone;

    m_subPassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    m_subPassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite; ;

}
