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

        m_colourAttachments.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        m_msaaAttachments.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);

        //==========================
        // CREATE DEPTH ATTACHMENT
        //==========================
        m_depthAttachment.second = std::make_unique<VulkanCore::VImage>(m_device, 1, m_device.GetDepthFormat(), vk::ImageAspectFlagBits::eDepth,
            vk::ImageUsageFlagBits::eDepthStencilAttachment, m_device.GetSampleCount());
        auto& depthAttachmentInfo = m_depthAttachment.first;
        depthAttachmentInfo.imageView = m_depthAttachment.second->GetImageView();
        depthAttachmentInfo.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachmentInfo.clearValue.depthStencil.depth = 1.0f;
        depthAttachmentInfo.clearValue.depthStencil.stencil = 0.0f;

        m_depthAttachment.second->Resize(width, height);


        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            //==========================
            // CREATE COLOUR ATTACHMENT
            //==========================
            m_colourAttachments[i].second = std::make_unique<VulkanCore::VImage>(m_device, 1, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor,
                vk::ImageUsageFlagBits::eColorAttachment| vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment);
            auto& colourAttachmentInfo  = m_colourAttachments[i].first;
            colourAttachmentInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            colourAttachmentInfo.imageView = m_colourAttachments[i].second->GetImageView();
            colourAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
            colourAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
            colourAttachmentInfo.clearValue.color.setFloat32({0.f, 0.f, 0.f, 1.f});

            m_colourAttachments[i].second->Resize(width, height);



            //==========================
            // CREATE MSAA ATTACHMENT
            //==========================
            m_msaaAttachments[i].second = std::make_unique<VulkanCore::VImage>(m_device, 1, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eColorAttachment| vk::ImageUsageFlagBits::eTransientAttachment, m_device.GetSampleCount() );
            auto& msaaAttachmentInfo  = m_msaaAttachments[i].first;
            msaaAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            msaaAttachmentInfo.imageView = m_msaaAttachments[i].second->GetImageView();
            msaaAttachmentInfo.resolveImageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            msaaAttachmentInfo.resolveImageView = colourAttachmentInfo.imageView;
            msaaAttachmentInfo.resolveMode = vk::ResolveModeFlagBits::eAverage;
            msaaAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
            msaaAttachmentInfo.clearValue.color.setFloat32({0.f, 0.f, 0.f, 1.f});
            msaaAttachmentInfo.storeOp = vk::AttachmentStoreOp::eDontCare;

            m_msaaAttachments[i].second->Resize(width, height);
        }


        auto transitionCommandBuffer = VulkanCore::VCommandBuffer(m_device,m_device.GetTransferCommandPool());
        auto transitionFinishedFence = VulkanCore::VSyncPrimitive<vk::Fence>(m_device);
        transitionCommandBuffer.BeginRecording();

        // TRANSITION EVERYTHING FROM UNDEFINED LAYOUT TO COLOUR ATTACHMENT
         m_colourAttachments[0].second->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, transitionCommandBuffer);
         m_colourAttachments[1].second->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, transitionCommandBuffer);

        m_msaaAttachments[0].second->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, transitionCommandBuffer);
        m_msaaAttachments[1].second->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, transitionCommandBuffer);


         m_depthAttachment.second->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, transitionCommandBuffer);

        transitionCommandBuffer.EndAndFlush(m_device.GetTransferQueue(), transitionFinishedFence.GetSyncPrimitive());

        transitionFinishedFence.WaitForFence(-1);

        transitionFinishedFence.Destroy();

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
        for (auto &colourBuffer : m_colourAttachments)
        {
            colourBuffer.second->Resize(newWidth, newHeight);
        }
        m_depthAttachment.second->Resize(newWidth, newHeight);
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
        for (int i = 0; i < m_colourAttachments.size(); i++)
        {
            m_colourAttachments[i].second->Destroy();
            if (m_msaaAttachments[i].second)
                m_msaaAttachments[i].second->Destroy();
        }
        m_depthAttachment.second->Destroy();

        Utils::Logger::LogSuccess("Render target destroyed");
    }

    void RenderTarget::DestroyForResize()
    {

    }

    vk::ImageView RenderTarget::GetColourImageView(int currentFrame) const
    {
    }

    vk::ImageView RenderTarget::GetDepthImageView() const
    {
    }

    vk::ImageView RenderTarget::GetResolveImageView() const
    {
    }

    vk::RenderingAttachmentInfo& RenderTarget::GetColourAttachment(int currentFrame)
    {
        return m_msaaAttachments[currentFrame].first;
    }

    vk::RenderingAttachmentInfo& RenderTarget::GetMSAAResolveAttachment(int currentFrame)
    {
        return m_colourAttachments[currentFrame].first;
    }

    vk::RenderingAttachmentInfo& RenderTarget::GetDepthAttachment()
    {
        return m_depthAttachment.first;
    }

    const VulkanCore::VImage& RenderTarget::GetColourImage(int currentFrame) const
    {
        return *m_colourAttachments[currentFrame].second;
    }

    const VulkanCore::VImage& RenderTarget::GetDepthImage(int currentFrame) const
    {
        return *m_depthAttachment.second;
    }

    const VulkanCore::VImage& RenderTarget::GetMSAAResolvedImage(int currentFrame) const
    {
        return *m_msaaAttachments[currentFrame].second;
    }

    void RenderTarget::CreateRenderTargetForSwapChain(const VulkanCore::VSwapChain& swapChain)
    {
        auto& swapChainImages = swapChain.GetSwapChainImages();
        auto& swapChainExtent = swapChain.GetExtent();
        auto& swapChainFormat = swapChain.GetSurfaceFormatKHR().format;
        // for swap chain
        Utils::Logger::LogInfoVerboseOnly("Creating render target for swap chain images...");
        m_colourAttachments.resize(swapChainImages.size());

        //==========================
        // CREATE DEPTH ATTACHMENT
        //==========================
        m_depthAttachment.second = std::make_unique<VulkanCore::VImage>(m_device, 1, m_device.GetDepthFormat(),
                                                             vk::ImageAspectFlagBits::eDepth, vk::ImageUsageFlagBits::eDepthStencilAttachment);

        auto& depthSwapChainAttachment = m_depthAttachment.first;
        depthSwapChainAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        depthSwapChainAttachment.clearValue.depthStencil.depth = 1.0f;
        depthSwapChainAttachment.clearValue.depthStencil.stencil = 0.0f;
        depthSwapChainAttachment.imageView = m_depthAttachment.second->GetImageView();
        depthSwapChainAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthSwapChainAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;

        m_depthAttachment.second->Resize(swapChainExtent.width, swapChainExtent.height);

        //==========================
        // CREATE COLOUR ATTACHMENT
        //==========================
        for (int i = 0; i < swapChainImages.size(); i++)
        {
            m_colourAttachments[i].second = std::make_unique<VulkanCore::VImage>(m_device, swapChainImages[i],
                                                                     swapChainExtent.width, swapChainExtent.height
                                                                     , 1,
                                                                     swapChainFormat);

            auto& swapChainImageAttachment= m_colourAttachments[i].first;
            swapChainImageAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            swapChainImageAttachment.imageView = m_colourAttachments[i].second->GetImageView();
            swapChainImageAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            swapChainImageAttachment.storeOp = vk::AttachmentStoreOp::eStore;

        }

        auto transitionCommandBuffer = VulkanCore::VCommandBuffer(m_device,m_device.GetTransferCommandPool());
        auto transitionFinishedFence = VulkanCore::VSyncPrimitive<vk::Fence>(m_device);
        transitionCommandBuffer.BeginRecording();

        for (int i = 0; i < swapChainImages.size(); i++)
        {
            m_colourAttachments[i].second->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, transitionCommandBuffer);
        }

        m_depthAttachment.second->TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, transitionCommandBuffer);

        transitionFinishedFence.WaitForFence(-1);
        transitionFinishedFence.Destroy();

        Utils::Logger::LogSuccess("Render target for swap chain created, contains:" + std::to_string(swapChainImages.size()) + "Swap chain images and 1 depth buffer" );

        // TODO: apply this to the rendering and to the pipeline creation process, optionally i can use shader objects instead of shader modules to specify descriptor layouts....
    }
} // Renderer