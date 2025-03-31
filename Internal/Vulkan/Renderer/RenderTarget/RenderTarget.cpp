//
// Created by wpsimon09 on 22/12/24.
//

#include "RenderTarget.hpp"

#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

#define STENCIL_ONE 0xFF
#define STENCIL_ZERO 0x00

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
        VulkanCore::VImage2CreateInfo depthAttachmentCreateInfo;
        depthAttachmentCreateInfo.format = m_device.GetDepthFormat();
        depthAttachmentCreateInfo.height = m_height;
        depthAttachmentCreateInfo.width = m_width;
        depthAttachmentCreateInfo.mipLevels = 1;
        depthAttachmentCreateInfo.aspecFlags = vk::ImageAspectFlagBits::eDepth;
        depthAttachmentCreateInfo.samples = m_device.GetSampleCount();
        depthAttachmentCreateInfo.imageUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

        m_depthAttachment.second = std::make_unique<VulkanCore::VImage2>(m_device, depthAttachmentCreateInfo);

        auto& depthAttachmentInfo = m_depthAttachment.first;
        depthAttachmentInfo.imageView = m_depthAttachment.second->GetImageView();
        depthAttachmentInfo.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachmentInfo.resolveMode = vk::ResolveModeFlagBits::eNone;
        depthAttachmentInfo.clearValue.depthStencil.depth = 1.0f;
        depthAttachmentInfo.clearValue.depthStencil.stencil = 1.0f;



        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {

            //==========================
            // CREATE COLOUR ATTACHMENTS
            //==========================

            VulkanCore::VImage2CreateInfo colourAttachmentCreateInfo;
            colourAttachmentCreateInfo.format = colourFormat;
            colourAttachmentCreateInfo.height = m_height;
            colourAttachmentCreateInfo.width = m_width;
            colourAttachmentCreateInfo.mipLevels = 1;
            colourAttachmentCreateInfo.aspecFlags = vk::ImageAspectFlagBits::eColor;
            colourAttachmentCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment| vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment;
            m_colourAttachments[i].second = std::make_unique<VulkanCore::VImage2>(m_device, colourAttachmentCreateInfo);

            auto& colourAttachmentInfo  = m_colourAttachments[i].first;
            colourAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            colourAttachmentInfo.imageView = m_colourAttachments[i].second->GetImageView();
            colourAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
            depthAttachmentInfo.resolveMode = vk::ResolveModeFlagBits::eNone;
            colourAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
            colourAttachmentInfo.clearValue.color.setFloat32({0.2f, 0.2f, 0.2f, 1.f});


            //==========================
            // CREATE MSAA ATTACHMENT
            //==========================
            colourAttachmentCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment| vk::ImageUsageFlagBits::eTransientAttachment;
            colourAttachmentCreateInfo.samples = m_device.GetSampleCount();
            m_msaaAttachments[i].second = std::make_unique<VulkanCore::VImage2>(m_device, colourAttachmentCreateInfo);

            auto& msaaAttachmentInfo  = m_msaaAttachments[i].first;
                msaaAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            msaaAttachmentInfo.imageView = m_msaaAttachments[i].second->GetImageView();
            msaaAttachmentInfo.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            msaaAttachmentInfo.resolveImageView = m_colourAttachments[i].second->GetImageView();
            msaaAttachmentInfo.resolveMode = vk::ResolveModeFlagBits::eAverage;
            msaaAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
            msaaAttachmentInfo.clearValue.color.setFloat32({0.2f, 0.2f, 0.2f, 1.f});
            msaaAttachmentInfo.storeOp = vk::AttachmentStoreOp::eDontCare;

        }

        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VulkanUtils::RecordImageTransitionLayoutCommand(*m_colourAttachments[i].second,
                                                            vk::ImageLayout::eColorAttachmentOptimal,
                                                            vk::ImageLayout::eUndefined,
                                                            m_device.GetTransferOpsManager().GetCommandBuffer());

            VulkanUtils::RecordImageTransitionLayoutCommand(*m_msaaAttachments[i].second,
                                                            vk::ImageLayout::eColorAttachmentOptimal,
                                                            vk::ImageLayout::eUndefined,
                                                            m_device.GetTransferOpsManager().GetCommandBuffer());
        }
        // TRANSITION EVERYTHING FROM UNDEFINED LAYOUT TO COLOUR ATTACHMENT

        VulkanUtils::RecordImageTransitionLayoutCommand(*m_depthAttachment.second,
                                                            vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                                            vk::ImageLayout::eUndefined,
                                                            m_device.GetTransferOpsManager().GetCommandBuffer());


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
        Utils::Logger::LogSuccess("Render targSkip the third pass if outlines don’t need blending.et for swap chain recreated");
    }

    void RenderTarget::Destroy()
    {
        Utils::Logger::LogInfoVerboseOnly("Destroying render target...");
        for (int i = 0; i < m_colourAttachments.size(); i++)
        {
            m_colourAttachments[i].second->Destroy();
            if (!m_forSwapChain)
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
        return m_colourAttachments[currentFrame].second->GetImageView();
    }

    vk::ImageView RenderTarget::GetDepthImageView() const
    {
        return m_depthAttachment.second->GetImageView();
    }

    vk::ImageView RenderTarget::GetResolveImageView(int currentFrame) const
    {
        return m_colourAttachments [currentFrame].second->GetImageView();
    }

    vk::RenderingAttachmentInfo& RenderTarget::GetColourAttachmentOneSample(int currentFrame)
    {
        return m_colourAttachments[currentFrame].first;
    }

    vk::RenderingAttachmentInfo& RenderTarget::GetColourAttachmentMultiSampled(int currentFrame)
    {
        return m_msaaAttachments[currentFrame].first;
    }

    vk::RenderingAttachmentInfo& RenderTarget::GetDepthAttachment()
    {
        return m_depthAttachment.first;
    }

    VulkanCore::VImage2& RenderTarget::GetColourImage(int currentFrame) const
    {
        return *m_colourAttachments[currentFrame].second;
    }

    VulkanCore::VImage2& RenderTarget::GetDepthImage(int currentFrame) const
    {
        return *m_depthAttachment.second;
    }

    VulkanCore::VImage2& RenderTarget::GetColourAttachmentMultiSampled(int currentFrame) const
    {
        return *m_msaaAttachments[currentFrame].second;
    }

    void RenderTarget::CreateRenderTargetForSwapChain(const VulkanCore::VSwapChain& swapChain)
    {


        auto& swapChainImages = swapChain.GetSwapChainImages();
        auto& swapChainExtent = swapChain.GetExtent();
        auto& swapChainFormat = swapChain.GetSurfaceFormatKHR().format;
        m_forSwapChain = true;
        // for swap chain
        Utils::Logger::LogInfoVerboseOnly("Creating render target for swap chain images...");
        m_colourAttachments.resize(swapChainImages.size());

        //==========================
        // CREATE DEPTH ATTACHMENT
        //==========================
        VulkanCore::VImage2CreateInfo depthAttachmentCreateInfo;
        depthAttachmentCreateInfo.format = m_device.GetDepthFormat();
        depthAttachmentCreateInfo.height = swapChainExtent.height;
        depthAttachmentCreateInfo.width = swapChainExtent.width;
        depthAttachmentCreateInfo.mipLevels = 1;
        depthAttachmentCreateInfo.aspecFlags = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        depthAttachmentCreateInfo.imageUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

        m_depthAttachment.second = std::make_unique<VulkanCore::VImage2>(m_device, depthAttachmentCreateInfo);

        auto& depthSwapChainAttachment = m_depthAttachment.first;
        depthSwapChainAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        depthSwapChainAttachment.clearValue.depthStencil.depth = 1.0f;
        depthSwapChainAttachment.clearValue.depthStencil.stencil = 0.0f;
        depthSwapChainAttachment.imageView = m_depthAttachment.second->GetImageView();


        depthSwapChainAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthSwapChainAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;


        //==========================
        // CREATE COLOUR ATTACHMENT
        //==========================
        VulkanCore::VImage2CreateInfo colourAttachmentCreateInfo;
        colourAttachmentCreateInfo.format = swapChainFormat;
        colourAttachmentCreateInfo.height = swapChainExtent.height;
        colourAttachmentCreateInfo.width = swapChainExtent.width;
        colourAttachmentCreateInfo.mipLevels = 1;
        colourAttachmentCreateInfo.aspecFlags = vk::ImageAspectFlagBits::eColor;

        for (int i = 0; i < swapChainImages.size(); i++)
        {
            m_colourAttachments[i].second = std::make_unique<VulkanCore::VImage2>(m_device, colourAttachmentCreateInfo, swapChainImages[i]);

            auto& swapChainImageAttachment= m_colourAttachments[i].first;
            swapChainImageAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            swapChainImageAttachment.imageView = m_colourAttachments[i].second->GetImageView();
            swapChainImageAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            swapChainImageAttachment.storeOp = vk::AttachmentStoreOp::eStore;

        }



        for (int i = 0; i < swapChainImages.size(); i++)
        {
            VulkanUtils::RecordImageTransitionLayoutCommand(*m_colourAttachments[i].second, vk::ImageLayout::eColorAttachmentOptimal,vk::ImageLayout::eUndefined, m_device.GetTransferOpsManager().GetCommandBuffer());
        }

        VulkanUtils::RecordImageTransitionLayoutCommand(*m_depthAttachment.second, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eUndefined ,m_device.GetTransferOpsManager().GetCommandBuffer());

        Utils::Logger::LogSuccess("Render target for swap chain created, contains:" + std::to_string(swapChainImages.size()) + "Swap chain images and 1 depth buffer" );

        // TODO: apply this to the rendering and to the pipeline creation process, optionally i can use shader objects instead of shader modules to specify descriptor layouts....
    }
} // Renderer