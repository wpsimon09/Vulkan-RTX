//
// Created by wpsimon09 on 06/10/24.
//

#include "VRenderPass.hpp"

#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"

VulkanCore::VRenderPass::VRenderPass(const VulkanCore::VDevice &device, const VulkanCore::VSwapChain &swapChain):m_device(device),m_swapChain(swapChain), VObject() {
    CreateRenderPass();
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
    m_colourAttachmentDescription.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    //-----------------
    // DEPTH ATTACHMENT
    //-----------------
    // WILL BE FILLED IN LATER

    //----------------------------------------------------------------------------------------------------------------------------
    // RESOLVE ATTACHEMENT
    // it transition the image format from colour attachment to the format that is suitable to present the images on the screen
    //----------------------------------------------------------------------------------------------------------------------------
    m_resolveColourAttachmentDescription.format =   m_swapChain.GetSurfaceFormatKHR().format;
    m_resolveColourAttachmentDescription.samples = vk::SampleCountFlagBits::e1;
    m_resolveColourAttachmentDescription.loadOp = vk::AttachmentLoadOp::eDontCare;
    m_resolveColourAttachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
    m_resolveColourAttachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    m_resolveColourAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    m_resolveColourAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
    m_resolveColourAttachmentDescription.finalLayout = vk::ImageLayout::ePresentSrcKHR;

}

void VulkanCore::VRenderPass::Destroy() {
    m_device.GetDevice().destroyRenderPass(m_renderPass);
}
