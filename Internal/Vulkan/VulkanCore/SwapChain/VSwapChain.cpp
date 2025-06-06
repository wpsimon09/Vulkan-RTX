//
// Created by wpsimon09 on 29/09/24.
//

#include "VSwapChain.hpp"

#include <complex>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Application/WindowManager/WindowManager.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"

VulkanCore::VSwapChain::VSwapChain(const VulkanCore::VDevice& device, const VulkanCore::VulkanInstance& instance)
    : m_device(device)
    , m_instance(instance)
{
    ChooseExtent();
    ChooseFormat();
    ChoosePresentMode();
    CreateSwapChain();
    RetrieveSwapChainImages();
}


void VulkanCore::VSwapChain::Destroy()
{
    Utils::Logger::LogInfoVerboseOnly("Destroying swap chain images... ");

    m_device.GetDevice().destroySwapchainKHR(m_swapChain);

    Utils::Logger::LogInfoVerboseOnly("Swap chain destroyed !");
}

void VulkanCore::VSwapChain::ChooseExtent()
{
    Utils::Logger::LogInfoVerboseOnly("Choosing right extend...");
    auto capabilities = m_device.GetPhysicalDevice().getSurfaceCapabilitiesKHR(m_instance.GetSurface());
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        m_extent = capabilities.currentExtent;
        Utils::Logger::LogInfoVerboseOnly("Going to use default swap chain extend");
        return;
    }

    vk::Extent2D acctualExtend = {
        static_cast<uint32_t>(WindowManager::GetWindowWidth()),
        static_cast<uint32_t>(WindowManager::GetWindowHeight()),
    };

    acctualExtend.width = std::clamp(acctualExtend.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    acctualExtend.height =
        std::clamp(acctualExtend.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    m_extent = acctualExtend;
    Utils::Logger::LogInfoVerboseOnly("Current extend of the window is going ot be set \n \t Width:: "
                                      + std::to_string(acctualExtend.width) + "\n"
                                      + "\tHeight:: " + std::to_string(acctualExtend.height));
}

void VulkanCore::VSwapChain::ChooseFormat()
{
    Utils::Logger::LogInfoVerboseOnly("Choosing surface format...");
    auto availableFormats = m_device.GetPhysicalDevice().getSurfaceFormatsKHR(m_instance.GetSurface());
    for(auto& format : availableFormats)
    {
        if(format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            m_format = format;
            break;
        }
    }
    m_format = availableFormats.front();
    Utils::Logger::LogInfoVerboseOnly("Format selected");
}

void VulkanCore::VSwapChain::ChoosePresentMode()
{
    Utils::Logger::LogInfoVerboseOnly("Choosing surface present mode...");
    auto availablePresentModes = m_device.GetPhysicalDevice().getSurfacePresentModesKHR(m_instance.GetSurface());
    for(auto& presentMode : availablePresentModes)
    {
        if(presentMode == vk::PresentModeKHR::eMailbox)
        {
            m_presentMode = presentMode;
            break;
        }
    }
    m_presentMode = vk::PresentModeKHR::eFifo;
    Utils::Logger::LogInfoVerboseOnly("Present mode selected");
}

void VulkanCore::VSwapChain::CreateSwapChain()
{
    Utils::Logger::LogInfoVerboseOnly("Creating swap chain...");

    auto capabilites   = m_device.GetPhysicalDevice().getSurfaceCapabilitiesKHR(m_instance.GetSurface());
    auto minImageCount = capabilites.minImageCount + 1;

    if(capabilites.maxImageCount > 0 && minImageCount > capabilites.maxImageCount)
    {
        minImageCount = capabilites.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChainCreateInfo;
    swapChainCreateInfo.surface          = m_instance.GetSurface();
    swapChainCreateInfo.minImageCount    = minImageCount;
    swapChainCreateInfo.imageFormat      = m_format.format; // vk::Format::eR16G16B16A16Sfloat
    swapChainCreateInfo.imageColorSpace  = m_format.colorSpace; //vk::ColorSpaceKHR::eSrgbNonLinear
    swapChainCreateInfo.imageExtent      = m_extent;
    swapChainCreateInfo.presentMode      = m_presentMode;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
    swapChainCreateInfo.preTransform     = capabilites.currentTransform; //vk:SurfaceTransformFlagBitsKHR::eldentity
    swapChainCreateInfo.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainCreateInfo.presentMode      = m_presentMode; // fifo
    swapChainCreateInfo.clipped          = VK_TRUE;
    swapChainCreateInfo.oldSwapchain     = VK_NULL_HANDLE;

    std::vector<uint32_t> queueFamilyIndices = {m_device.GetConcreteQueueFamilyIndex(EQueueFamilyIndexType::Graphics),
                                                m_device.GetConcreteQueueFamilyIndex(EQueueFamilyIndexType::PresentKHR)};

    //graphics vs transfer
    if(queueFamilyIndices[0] != queueFamilyIndices[1])
    {
        swapChainCreateInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        swapChainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
    }
    else
    {
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    m_swapChain = m_device.GetDevice().createSwapchainKHR(swapChainCreateInfo, nullptr);
    assert(m_swapChain != VK_NULL_HANDLE); // assertion triggered
    Utils::Logger::LogSuccess("Swap chain created");
}

void VulkanCore::VSwapChain::RetrieveSwapChainImages()
{
    m_swapChainImages = m_device.GetDevice().getSwapchainImagesKHR(m_swapChain);
}


void VulkanCore::VSwapChain::RecreateSwapChain()
{
    Utils::Logger::LogInfo("Recreating swap chain...");
    m_device.GetDevice().waitIdle();

    Destroy();
    ChooseExtent();
    ChooseFormat();
    ChoosePresentMode();
    CreateSwapChain();
    RetrieveSwapChainImages();


    Utils::Logger::LogSuccess("Swap chain recreated ");
}
