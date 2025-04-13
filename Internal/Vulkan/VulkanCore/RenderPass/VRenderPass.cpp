//
// Created by wpsimon09 on 06/10/24.
//

#include "VRenderPass.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


VulkanCore::VRenderPass::VRenderPass(const VulkanCore::VDevice& device,
                                     const VulkanCore::VImage&  colourBuffer,
                                     const VulkanCore::VImage&  depthBuffer,
                                     const VulkanCore::VImage&  msaaBuffer,
                                     bool                       ForSwapChain)
    : m_device(device)
    , m_colourBuffer(colourBuffer)
    , m_depthBuffer(depthBuffer)
    , m_msaaBuffer(msaaBuffer)
{
  Utils::Logger::LogInfoVerboseOnly("Creating render pass...");
  m_forSwapChain = ForSwapChain;

  if(ForSwapChain)
  {
    CreateRenderPassForSwapChain();
  }
  else
  {
    CreateRenderPassForCustomImage();
  }
}

void VulkanCore::VRenderPass::Destroy()
{
  Utils::Logger::LogInfoVerboseOnly("Render pass destoryed");
  m_device.GetDevice().destroyRenderPass(m_renderPass);
}

void VulkanCore::VRenderPass::CreateRenderPassForSwapChain()
{

  //------------------------
  // BASIC COLOUR ATTACHMENT
  //------------------------
  m_colourAttachmentDescription.format         = m_colourBuffer.GetFormat();
  m_colourAttachmentDescription.samples        = vk::SampleCountFlagBits::e1;
  m_colourAttachmentDescription.loadOp         = vk::AttachmentLoadOp::eClear;
  m_colourAttachmentDescription.storeOp        = vk::AttachmentStoreOp::eStore;
  m_colourAttachmentDescription.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
  m_colourAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  m_colourAttachmentDescription.initialLayout  = vk::ImageLayout::eUndefined;
  m_colourAttachmentDescription.finalLayout    = vk::ImageLayout::ePresentSrcKHR;

  m_colourAttachmentRef.attachment = 0;
  m_colourAttachmentRef.layout     = vk::ImageLayout::ePresentSrcKHR;

  //-----------------
  // DEPTH ATTACHMENT
  //-----------------
  m_depthStencilAttachmentDescription.format         = m_depthBuffer.GetFormat();
  m_depthStencilAttachmentDescription.samples        = vk::SampleCountFlagBits::e1;
  m_depthStencilAttachmentDescription.loadOp         = vk::AttachmentLoadOp::eClear;
  m_depthStencilAttachmentDescription.storeOp        = vk::AttachmentStoreOp::eDontCare;
  m_depthStencilAttachmentDescription.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
  m_depthStencilAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  m_depthStencilAttachmentDescription.initialLayout  = vk::ImageLayout::eUndefined;
  m_depthStencilAttachmentDescription.finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;

  m_depthStencilAttachmentRef.attachment = 1;
  m_depthStencilAttachmentRef.layout     = vk::ImageLayout::eDepthStencilAttachmentOptimal;


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
  renderPassInfo.attachmentCount          = attachments.size();
  renderPassInfo.pAttachments             = attachments.data();
  renderPassInfo.subpassCount             = 1;
  renderPassInfo.pSubpasses               = &m_subPass;
  renderPassInfo.dependencyCount          = 1;
  renderPassInfo.pDependencies            = &m_subPassDependency;

  m_renderPass = m_device.GetDevice().createRenderPass(renderPassInfo);
  assert(m_renderPass);
  Utils::Logger::LogSuccess("Render pass created");
}

void VulkanCore::VRenderPass::CreateRenderPassForCustomImage()
{
  //------------------------
  // BASIC COLOUR ATTACHMENT
  //------------------------
  m_colourAttachmentDescription.format         = m_colourBuffer.GetFormat();
  m_colourAttachmentDescription.samples        = m_device.GetSampleCount();
  m_colourAttachmentDescription.loadOp         = vk::AttachmentLoadOp::eClear;
  m_colourAttachmentDescription.storeOp        = vk::AttachmentStoreOp::eStore;
  m_colourAttachmentDescription.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
  m_colourAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  //assert(m_colourBuffer.GetCurrentLayout() == vk::ImageLayout::eColorAttachmentOptimal && "Color buffer must be in color attachment optimal");
  m_colourAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
  m_colourAttachmentDescription.finalLayout   = vk::ImageLayout::eColorAttachmentOptimal;

  m_colourAttachmentRef.attachment = 0;
  m_colourAttachmentRef.layout     = vk::ImageLayout::eColorAttachmentOptimal;

  //-----------------
  // DEPTH ATTACHMENT
  //-----------------
  m_depthStencilAttachmentDescription.format         = m_depthBuffer.GetFormat();
  m_depthStencilAttachmentDescription.samples        = m_device.GetSampleCount();
  m_depthStencilAttachmentDescription.loadOp         = vk::AttachmentLoadOp::eDontCare;
  m_depthStencilAttachmentDescription.storeOp        = vk::AttachmentStoreOp::eDontCare;
  m_depthStencilAttachmentDescription.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
  m_depthStencilAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  //assert(m_depthBuffer.GetCurrentLayout() == vk::ImageLayout::eDepthStencilAttachmentOptimal && "Color buffer must be in color attachment optimal");
  m_depthStencilAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
  m_depthStencilAttachmentDescription.finalLayout   = vk::ImageLayout::eDepthStencilAttachmentOptimal;

  m_depthStencilAttachmentRef.attachment = 1;
  m_depthStencilAttachmentRef.layout     = vk::ImageLayout::eDepthStencilAttachmentOptimal;


  //----------------------------------------------------------------------------------------------------------------------------
  // RESOLVE ATTACHEMENT
  // it transition the image format from colour attachment to the format that is suitable to present the images on the screen
  //----------------------------------------------------------------------------------------------------------------------------

  m_resolveColourAttachmentDescription.format         = m_colourBuffer.GetFormat();
  m_resolveColourAttachmentDescription.samples        = vk::SampleCountFlagBits::e1;
  m_resolveColourAttachmentDescription.loadOp         = vk::AttachmentLoadOp::eDontCare;
  m_resolveColourAttachmentDescription.storeOp        = vk::AttachmentStoreOp::eStore;
  m_resolveColourAttachmentDescription.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
  m_resolveColourAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  m_resolveColourAttachmentDescription.initialLayout  = vk::ImageLayout::eUndefined;
  m_resolveColourAttachmentDescription.finalLayout    = vk::ImageLayout::eColorAttachmentOptimal;

  m_resolveColourAttachmentRef.attachment = 2;
  m_resolveColourAttachmentRef.layout     = vk::ImageLayout::eColorAttachmentOptimal;
  CreateMainSubPass();

  std::array<vk::AttachmentDescription, 3> attachments = {m_colourAttachmentDescription, m_depthStencilAttachmentDescription,
                                                          m_resolveColourAttachmentDescription};
  vk::RenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.attachmentCount          = attachments.size();
  renderPassInfo.pAttachments             = attachments.data();
  renderPassInfo.subpassCount             = 1;
  renderPassInfo.pSubpasses               = &m_subPass;
  renderPassInfo.dependencyCount          = 1;
  renderPassInfo.pDependencies            = &m_subPassDependency;


  m_renderPass = m_device.GetDevice().createRenderPass(renderPassInfo);
  assert(m_renderPass);
  Utils::Logger::LogSuccess("Render pass created");
}

void VulkanCore::VRenderPass::CreateMainSubPass()
{
  m_subPass.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics;
  m_subPass.colorAttachmentCount    = 1;
  m_subPass.pColorAttachments       = &m_colourAttachmentRef;
  m_subPass.pDepthStencilAttachment = &m_depthStencilAttachmentRef;

  if(!m_forSwapChain)
    m_subPass.pResolveAttachments = &m_resolveColourAttachmentRef;


  m_subPassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  m_subPassDependency.dstSubpass = 0;

  m_subPassDependency.srcStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  m_subPassDependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

  // Access masks for the transition
  m_subPassDependency.dstAccessMask = vk::AccessFlagBits::eShaderRead;
  m_subPassDependency.dstStageMask  = vk::PipelineStageFlagBits::eFragmentShader;

  // Ensure dependency flags for synchronization
  //m_subPassDependency.dependencyFlags = vk::DependencyFlagBits::eByRegion;
}
