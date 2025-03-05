//
// Created by wpsimon09 on 22/12/24.
//

#include "RenderTarget.hpp"

#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"


namespace Renderer {
    RenderTarget::RenderTarget(const VulkanCore::VDevice& device, int width, int height, vk::Format colourFormat):
        m_device(device), m_width(width), m_height(height)
    {
        // for custom attachemnt

        Utils::Logger::LogInfoVerboseOnly("Creating render target...");

        m_colourBuffer.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_frameBuffers.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);

        //==========================
        // CREATE DEPTH ATTACHMENT
        //==========================
        m_depthBuffer = std::make_unique<VulkanCore::VImage>(m_device, 1, m_device.GetDepthFormat(), vk::ImageAspectFlagBits::eDepth,
            vk::ImageUsageFlagBits::eDepthStencilAttachment, m_device.GetSampleCount());

        m_depthBuffer->Resize(width, height);

        //==========================
        // CREATE MSAA ATTACHMENT
        //==========================
        m_msaaBuffer = std::make_unique<VulkanCore::VImage>(m_device, 1, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor,
                vk::ImageUsageFlagBits::eColorAttachment| vk::ImageUsageFlagBits::eTransientAttachment, m_device.GetSampleCount() );

        m_depthAttachments.second = std::make_unique<VulkanCore::VImage>(m_device, 1, m_device.GetDepthFormat(), vk::ImageAspectFlagBits::eDepth,
            vk::ImageUsageFlagBits::eDepthStencilAttachment, m_device.GetSampleCount());
        auto& depthAttachmentInfo = m_depthAttachments.first;
        depthAttachmentInfo.imageView = m_depthAttachments.second->GetImageView();
        depthAttachmentInfo.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachmentInfo.clearValue.depthStencil.depth = 1.0f;
        depthAttachmentInfo.clearValue.depthStencil.stencil = 0.0f;

        //==========================
        // CREATE COLOUR ATTACHMENT
        //==========================
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_colourAttachemnts[i].second = std::make_unique<VulkanCore::VImage>(m_device, 1, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor,
                vk::ImageUsageFlagBits::eColorAttachment| vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment);
            auto& colourAttachmentInfo  = m_colourAttachemnts[i].first;
            colourAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            colourAttachmentInfo.imageView = m_colourAttachemnts[i].second->GetImageView();
            colourAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
            colourAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
            colourAttachmentInfo.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

            m_colourAttachemnts[i].second->Resize(width, height);


            m_msaaAttachment[i].second = std::make_unique<VulkanCore::VImage>(m_device, 1, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eColorAttachment| vk::ImageUsageFlagBits::eTransientAttachment, m_device.GetSampleCount() );
            auto& msaaAttachmentInfo  = m_msaaAttachment[i].first;
            msaaAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            msaaAttachmentInfo.imageView = m_msaaAttachment[i].second->GetImageView();
            msaaAttachmentInfo.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            msaaAttachmentInfo.resolveImageView = colourAttachmentInfo.imageView;
            msaaAttachmentInfo.resolveMode = vk::ResolveModeFlagBits::eAverage;
            msaaAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
            msaaAttachmentInfo.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            msaaAttachmentInfo.storeOp = vk::AttachmentStoreOp::eDontCare;

            m_msaaAttachment[i].second->Resize(width, height);
        }


        //==========================
        // CREATE RENDER PASS
        //==========================
        m_renderPass = std::make_unique<VulkanCore::VRenderPass>(m_device,*m_colourBuffer[0],*m_depthBuffer ,*m_msaaBuffer, false);

        //==========================
        // CREATE FRAME BUFFERS
        //==========================
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::vector<std::reference_wrapper<const VulkanCore::VImage>> attachments;
            attachments.emplace_back(*m_msaaBuffer);
            attachments.emplace_back(*m_depthBuffer);
            attachments.emplace_back(*m_colourBuffer[i]);

            m_frameBuffers[i] = std::make_unique<VulkanCore::VFrameBuffer>(m_device, *m_renderPass,attachments, width, height);
        }

        auto transitionCommandBuffer = VulkanCore::VCommandBuffer(m_device,m_device.GetTransferCommandPool());

        // TRANSITION EVERYTHING FROM UNDEFINED LAYOUT TO COLOUR ATTACHMENT
         m_colourBuffer[0]->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, transitionCommandBuffer);
         m_colourBuffer[1]->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, transitionCommandBuffer);

         m_depthBuffer->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, transitionCommandBuffer);



        Utils::Logger::LogSuccess("Render target created, Contains 2 colour buffers and 1 depth buffer");
    }

    RenderTarget::RenderTarget(const VulkanCore::VDevice& device,const VulkanCore::VSwapChain& swapChain): m_device(device), m_width(swapChain.GetExtent().width), m_height(swapChain.GetExtent().height)
    {
        CreateRenderTargetForSwapChain(swapChain);
        Utils::Logger::LogSuccess("Render target for swap chain created successfuly !");
    }

    void RenderTarget::HandleResize(int newWidth, int newHeight)
    {
        m_width = newWidth;
        m_height = newHeight;
        m_device.GetDevice().waitIdle();
        DestroyForResize();
        for (auto &colourBuffer : m_colourBuffer)
        {
            colourBuffer->Resize(newWidth, newHeight);
        }
        m_depthBuffer->Resize(newWidth, newHeight);
    }

    void RenderTarget::HandleSwapChainResize(const VulkanCore::VSwapChain& swapChain)
    {
        Destroy();
        CreateRenderTargetForSwapChain(swapChain);
        m_width = swapChain.GetExtent().width;
        m_height = swapChain.GetExtent().height;
        Utils::Logger::LogSuccess("Render target for swap chain recreated");
    }

    void RenderTarget::Destroy()
    {
        Utils::Logger::LogInfoVerboseOnly("Destroying render target...");
        for (int i = 0; i < m_colourBuffer.size(); i++)
        {
            m_colourBuffer[i]->Destroy();
        }
        for (int i = 0; i < m_frameBuffers.size(); i++)
        {
            m_frameBuffers[i]->Destroy();
        }
        m_depthBuffer->Destroy();
        m_renderPass->Destroy();
        if (m_msaaBuffer)
            m_msaaBuffer->Destroy();

        Utils::Logger::LogSuccess("Render target destroyed");
    }

    void RenderTarget::DestroyForResize()
    {
        for (auto& frameBuffer : m_frameBuffers)
        {
            frameBuffer->Destroy();
        }
        m_renderPass->Destroy();
    }

    void RenderTarget::CreateRenderTargetForSwapChain(const VulkanCore::VSwapChain& swapChain)
    {
        auto swapChainImages = swapChain.GetSwapChainImages();
        auto swapChainExtent = swapChain.GetExtent();
        auto swapChainFormat = swapChain.GetSurfaceFormatKHR().format;
        // for swap chain
        Utils::Logger::LogInfoVerboseOnly("Creating render target for swap chain images...");
        m_colourBuffer.resize(swapChainImages.size());
        m_frameBuffers.resize(swapChainImages.size());


        //==========================
        // CREATE DEPTH ATTACHMENT
        //==========================
        m_depthBuffer = std::make_unique<VulkanCore::VImage>(m_device, 1, m_device.GetDepthFormat(),
                                                             vk::ImageAspectFlagBits::eDepth, vk::ImageUsageFlagBits::eDepthStencilAttachment);
        m_depthBuffer->Resize(swapChainExtent.width, swapChainExtent.height);

        //==========================
        // CREATE COLOUR ATTACHMENT
        //==========================
        for (int i = 0; i < swapChainImages.size(); i++)
        {
            m_colourBuffer[i] = std::make_unique<VulkanCore::VImage>(m_device, swapChainImages[i],
                                                                     swapChainExtent.width, swapChainExtent.height
                                                                     , 1,
                                                                     swapChainFormat);
        }

        m_renderPass = std::make_unique<VulkanCore::VRenderPass>(m_device,*m_colourBuffer[0],*m_depthBuffer, *m_msaaBuffer ,true);

        for (int i = 0; i < swapChainImages.size(); i++)
        {
            std::vector<std::reference_wrapper<const VulkanCore::VImage>> attachments;
            attachments.emplace_back(*m_colourBuffer[i]);
            attachments.emplace_back(*m_depthBuffer);
            m_frameBuffers[i] = std::make_unique<VulkanCore::VFrameBuffer>(m_device, *m_renderPass,attachments, swapChainExtent.width, swapChainExtent.height);
        }
    }
} // Renderer